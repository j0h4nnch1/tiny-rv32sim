#include "core/cpu.h"

uint32_t count = 0;
uint32_t cpu_t::decode_run(riscv32_cpu_state& state, instr data, mmu_t& iv_mem, uint32_t time_sed, uart& iv_uart, decode& iv_decode){
#if 0
    for(auto it : all_inst){
        if((data.val&it._mask)==it._match){
            it.func(state, data);
        }
    }
#endif

	uint32_t new_timer = state.timerl + time_sed;
	if( new_timer < state.timerl) state.timerh++;
	state.timerl = new_timer;

	// Handle Timer interrupt.
    if((state.timerh > state.timermatchh || (state.timerh == state.timermatchh && state.timerl > state.timermatchl))&& (state.timermatchh || state.timermatchl)){
		state.extraflags &= ~4; // Clear WFI
		state.mip |= 1<<7; //MTIP of MIP // https://stackoverflow.com/a/61916199/2926815  Fire interrupt.
	}
	else
		state.mip &= ~(1<<7);
    
    if(state.extraflags & 4 == 1){
        printf("WFI!\n");
        return 1;
        //WFI cpu should not run when wait for interpert
    }

    uint32_t ir = data.val;
    uint32_t trap = 0, pre_rd = 0, cycle = 0;
    uint32_t rd_idx = (ir >> 7) & 0x1f;
    int32_t ret_val = -1;

    if((state.mip & (1<<7)) && (state.mie & (1<<7) /*mtie*/ ) && (state.mstatus & 0x8 /*mie*/)){
		trap = 0x80000007;
		state.pc -= 4;
	}
    else{
        iv_decode.decode_run_without_trap(ir, trap, pre_rd, rd_idx, state, ret_val, iv_uart, iv_mem);
        if(ret_val != -1){
            return ret_val;
        }
    }//else

    // If there was a trap, do NOT allow register writeback.
    if( trap ){
        if(trap & 0x80000000){// a interpert
            state.mcause = trap;
            state.mtval = 0;
            handle_exception(state, ir, trap);
            state.pc += 4;
        }
        else{
            state.mcause = trap - 1;
            state.mtval = (trap > 5 && trap <= 8)? pre_rd : state.pc;
        }
        state.mepc = state.pc;
        // state.mepc += 4;
        state.mstatus = (( state.mstatus & 0x08) << 4) | ((state.extraflags & 3 ) << 11);
		state.pc = (state.mtvec - 4);


		// If trapping, always enter machine mode.
		state.extraflags |= 3;

		trap = 0;
		state.pc += 4;
        return 0;
    }
    state.pc += 4;
    if( rd_idx )//zero reg should not be written any value
    {
        state.gpr[rd_idx].val = pre_rd;
    }
    return 0;
}

void cpu_t::dump_state(riscv32_cpu_state& state, instr data){
    printf("PC: %08x [0x%08x] ", state.pc, data.val);
    printf( "Z:%08x ra:%08x sp:%08x gp:%08x tp:%08x t0:%08x t1:%08x t2:%08x s0:%08x s1:%08x a0:%08x a1:%08x a2:%08x a3:%08x a4:%08x a5:%08x ",
    state.gpr[0].val, state.gpr[1].val, state.gpr[2].val, state.gpr[3].val, state.gpr[4].val, state.gpr[5].val, state.gpr[6].val, state.gpr[7].val,
    state.gpr[8].val, state.gpr[9].val, state.gpr[10].val, state.gpr[11].val, state.gpr[12].val, state.gpr[13].val, state.gpr[14].val, state.gpr[15].val );
	printf( "a6:%08x a7:%08x s2:%08x s3:%08x s4:%08x s5:%08x s6:%08x s7:%08x s8:%08x s9:%08x s10:%08x s11:%08x t3:%08x t4:%08x t5:%08x t6:%08x\n",
    state.gpr[16].val, state.gpr[17].val, state.gpr[18].val, state.gpr[19].val, state.gpr[20].val, state.gpr[21].val, state.gpr[22].val, state.gpr[23].val,
    state.gpr[24].val, state.gpr[25].val, state.gpr[26].val, state.gpr[27].val, state.gpr[28].val, state.gpr[29].val, state.gpr[30].val, state.gpr[31].val );
    printf("IR:%08x\n", data.val);
}

uint32_t cpu_t::handle_exception(riscv32_cpu_state& state, uint32_t ir, uint32_t trap){
    //todo 
    return trap;
}

void cpu_t::register_func(decoder decode){
    all_inst.push_back(decode);
}

void cpu_t:: init(riscv32_cpu_state& cpu){
    //zero reg x0, and set pc
    
    cpu.gpr[0].val = 0;
    cpu.pc = RESET_VECTOR;
#if 0
#define DECLARE_INSN(name, match, mask) \
    uint32_t name##_match = (match); \
    uint32_t name##_mask = (mask); \
    extern uint32_t func_##name(riscv32_cpu_state&, instr); \

#include "core/encoding.h"
#undef DECLARE_INSN

#define DECLARE_FUNC(name) \
    register_func((decoder){ \
        name##_mask, \
        name##_match, \
        func_##name \
    });
#include "core/instruction_list.h"

#undef DECLARE_FUNC
#endif
}

uint64_t cpu_t::GetTimeMicroseconds(){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}


pc_t cpu_t::fetch_decode_exec(mmu_t& iv_mem, riscv32_cpu_state& cpu, uart& iv_uart, decode& iv_decode){

    uint32_t max_inst = -1;
    cpu.extraflags |= 3;//machine module
    uint32_t last_time = GetTimeMicroseconds();
    uint32_t single_step = 0;
    for(uint32_t cycle = 0; cycle < max_inst||max_inst < 0; cycle++){
        instr isn;
        uint32_t time_sed = GetTimeMicroseconds()-last_time;
        last_time += time_sed;
        isn.val = iv_mem.fetch(cpu.pc);
        count++;
        if(single_step){
            dump_state(cpu, isn);
        }
        uint32_t ret = decode_run(cpu, isn, iv_mem, time_sed, iv_uart, iv_decode);

        switch(ret){
            case 0: break;
            case 1:{
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                break;
                //todo: should halt here
            }
            case 2: 
                printf("return 2 fault\n");
                max_inst = 0;//stop when fault
                break;
            case 3:
                printf("program stop normally!\n");   
                max_inst = 0;
                break;
            default:
                printf("unknown fault!\n");
                break;
        }
    }

    return cpu.pc;
}