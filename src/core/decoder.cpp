#include <cstdint>
#include "core/decoder.h"

void decode::decode_run_without_trap(uint32_t ir, uint32_t& trap, uint32_t& pre_rd, uint32_t& rd_idx, riscv32_cpu_state& state, int32_t& ret_val, uart& iv_uart, mmu_t& iv_mem){
    switch( ir & 0x7f )
    {
        case 0x37: // LUI (0b0110111)
        {
            pre_rd = ( ir & 0xfffff000 );
            break;
        }
        case 0x17: // AUIPC (0b0010111)
        {
            pre_rd = state.pc + ( ir & 0xfffff000 );
            break;
        }
        case 0x6F: // JAL (0b1101111)
        {
            int32_t reladdy = ((ir & 0x80000000)>>11) | ((ir & 0x7fe00000)>>20) | ((ir & 0x00100000)>>9) | ((ir&0x000ff000));
            if( reladdy & 0x00100000 ) reladdy |= 0xffe00000; // Sign extension.
            pre_rd = state.pc + 4;
            state.pc = state.pc + reladdy - 4;
            break;
        }
        case 0x67: // JALR (0b1100111)
        {
            uint32_t imm = ir >> 20;
            int32_t imm_se = imm | (( imm & 0x800 )?0xfffff000:0);
            pre_rd = state.pc + 4;
            state.pc = ( (state.gpr[( (ir >> 15) & 0x1f )].val + imm_se) & ~1) - 4;
            break;
        }
        case 0x63: // Branch (0b1100011)
        {
            uint32_t immm4 = ((ir & 0xf00)>>7) | ((ir & 0x7e000000)>>20) | ((ir & 0x80) << 4) | ((ir >> 31)<<12);
            if( immm4 & 0x1000 ) immm4 |= 0xffffe000;
            int32_t rs1 = state.gpr[(ir >> 15) & 0x1f].val;
            int32_t rs2 = state.gpr[((ir >> 20) & 0x1f)].val;
            immm4 = state.pc + immm4 - 4;
            rd_idx = 0;
            switch( ( ir >> 12 ) & 0x7 )
            {
                // BEQ, BNE, BLT, BGE, BLTU, BGEU
                case 0: if( rs1 == rs2 ) state.pc = immm4; break;
                case 1: if( rs1 != rs2 ) state.pc = immm4; break;
                case 4: if( rs1 < rs2 ) state.pc = immm4; break;
                case 5: if( rs1 >= rs2 ) state.pc = immm4; break; //BGE
                case 6: if( (uint32_t)rs1 < (uint32_t)rs2 ) state.pc = immm4; break;   //BLTU
                case 7: if( (uint32_t)rs1 >= (uint32_t)rs2 ) state.pc = immm4; break;  //BGEU
                default: trap = (2+1);
            }
            break;
        }
        case 0x03: // Load (0b0000011)
        {
            uint32_t rs1 = state.gpr[((ir >> 15) & 0x1f)].val;
            uint32_t imm = ir >> 20;
            int32_t imm_se = imm | (( imm & 0x800 )?0xfffff000:0);
            uint32_t rsval = rs1 + imm_se;

            if( rsval >= MEM_RANGE || rsval < VADDR_BASE){   
                if( rsval >= 0x10000000 && rsval < 0x12000000 ){
                    if( rsval == 0x1100bffc )
                        pre_rd = state.timerh;
                    else if( rsval == 0x1100bff8 )
                        pre_rd = state.timerl;
                    else
                        pre_rd = iv_uart.HandleControlLoad(rsval);
                }
                else{
                    trap = (5+1);
                    pre_rd = rsval;
                }
            }
            else{
                switch( ( ir >> 12 ) & 0x7 ){
                    //LB, LH, LW, LBU, LHU
                    case 0: pre_rd = iv_mem.load(rsval, 1, true); break;
                    case 1: pre_rd = iv_mem.load(rsval, 2, true); break;
                    case 2: 
                        pre_rd = iv_mem.load(rsval, 4, false);
                        break;
                    case 4: pre_rd = iv_mem.load(rsval, 1, false); break;
                    case 5: pre_rd = iv_mem.load(rsval, 2, false); break;
                    default: trap = (2+1);
                }

            }
            break;
        }
        case 0x23: // Store 0b0100011
        {
            uint32_t rs1 = state.gpr[((ir >> 15) & 0x1f)].val;
            uint32_t rs2 = state.gpr[((ir >> 20) & 0x1f)].val;
            uint32_t addy = ( ( ir >> 7 ) & 0x1f ) | ( ( ir & 0xfe000000 ) >> 20 );
            if( addy & 0x800 ) addy |= 0xfffff000;
            addy += rs1;
            rd_idx = 0;
            if( addy >= MEM_RANGE || addy < VADDR_BASE)
            {
                if( addy >= 0x10000000 && addy < 0x12000000 ){
                    if( addy == 0x11004004 ){ //CLNT
                        state.timermatchh = rs2;
                    }
                    else if( addy == 0x11004000 ){ //CLNT
                        state.timermatchl = rs2;
                    }

                    else if( addy == 0x11100000 ){ //SYSCON (reboot, poweroff, etc.)
                        state.pc += 4;
                        ret_val = rs2;
                    }
                    else{
                        iv_uart.HandleControlStore(addy, rs2);
                    }
                        
                }
                else
                {
                    trap = (7+1); // Store access fault.
                    pre_rd = addy;
                }
            }
            else
            {
                switch( ( ir >> 12 ) & 0x7 )
                {
                    //SB, SH, SW
                    case 0: iv_mem.store( addy, rs2, 1); break;
                    case 1: iv_mem.store( addy, rs2, 2); break;
                    case 2: iv_mem.store( addy, rs2, 4); break;
                    default: trap = (2+1);
                }

            }

            break;
        }
        case 0x13: // Op-immediate 0b0010011
        case 0x33: // Op           0b0110011
        {
            uint32_t imm = ir >> 20;
            imm = imm | (( imm & 0x800 )?0xfffff000:0);
            uint32_t rs1 = state.gpr[((ir >> 15) & 0x1f)].val;
            uint32_t is_reg = !!( ir & 0x20 );
            uint32_t rs2 = is_reg ? state.gpr[(imm & 0x1f)].val : imm;

            if( is_reg && ( ir & 0x02000000 ) )
            {
                switch( (ir>>12)&7 ) //0x02000000 = RV32M
                {
                    case 0: pre_rd = rs1 * rs2; break; // MUL
#ifndef CUSTOM_MULH // If compiling on a system that doesn't natively, or via libgcc support 64-bit math.
                    case 1: pre_rd = ((int64_t)((int32_t)rs1) * (int64_t)((int32_t)rs2)) >> 32; break; // MULH
                    case 2: pre_rd = ((int64_t)((int32_t)rs1) * (uint64_t)rs2) >> 32; break; // MULHSU
                    case 3: pre_rd = ((uint64_t)rs1 * (uint64_t)rs2) >> 32; break; // MULHU
#else
                    CUSTOM_MULH
#endif
                    case 4: if( rs2 == 0 ) pre_rd = -1; else pre_rd = ((int32_t)rs1 == INT32_MIN && (int32_t)rs2 == -1) ? rs1 : ((int32_t)rs1 / (int32_t)rs2); break; // DIV
                    case 5: if( rs2 == 0 ) pre_rd = 0xffffffff; else pre_rd = rs1 / rs2; break; // DIVU
                    case 6: if( rs2 == 0 ) pre_rd = rs1; else pre_rd = ((int32_t)rs1 == INT32_MIN && (int32_t)rs2 == -1) ? 0 : ((uint32_t)((int32_t)rs1 % (int32_t)rs2)); break; // REM
                    case 7: if( rs2 == 0 ) pre_rd = rs1; else pre_rd = rs1 % rs2; break; // REMU
                }
            }
            else
            {
                switch( (ir>>12)&7 ) // These could be either op-immediate or op commands.  Be careful.
                {
                    case 0: pre_rd = (is_reg && (ir & 0x40000000) ) ? ( rs1 - rs2 ) : ( rs1 + rs2 ); break; 
                    case 1: pre_rd = rs1 << (rs2 & 0x1F); break;
                    case 2: pre_rd = (int32_t)rs1 < (int32_t)rs2; break;
                    case 3: pre_rd = rs1 < rs2; break;
                    case 4: pre_rd = rs1 ^ rs2; break;
                    case 5: pre_rd = (ir & 0x40000000 ) ? ( ((int32_t)rs1) >> (rs2 & 0x1F) ) : ( rs1 >> (rs2 & 0x1F) ); break;
                    case 6: pre_rd = rs1 | rs2; break;
                    case 7: pre_rd = rs1 & rs2; break;
                }
            }
            break;
        }
        case 0x0f: // 0b0001111
        {
            rd_idx = 0;   // fencetype = (ir >> 12) & 0b111;
            break;
        }
        // https://raw.githubusercontent.com/riscv/virtual-memory/main/specs/663-Svpbmt.pdf
        case 0x73: // Zifencei+Zicsr  (0b1110011)
        {
            uint32_t csrno = ir >> 20;//[31:20] is machine trap num
            uint32_t microop = ( ir >> 12 ) & 0x7;//which aotic operation
            if( (microop & 3) ) // It's a Zicsr function.
            {
                int rs1imm = (ir >> 15) & 0x1f;
                uint32_t rs1 = state.gpr[(rs1imm)].val;
                uint32_t writeval = rs1;
                
                switch( csrno )
                {
                    case 0x340: pre_rd = state. mscratch; break;
                    case 0x305: pre_rd = state. mtvec; break;
                    case 0x304: pre_rd = state.mie; break;
                    // case 0xC00: pre_rd = cycle; break;
                    case 0x344: pre_rd = state.mip; break;
                    case 0x341: pre_rd = state.mepc; break;
                    case 0x300: pre_rd = state.mstatus; break; //mstatus
                    case 0x342: pre_rd = state. mcause; break;
                    case 0x343: pre_rd = state. mtval; break;
                    case 0xf11: pre_rd = 0xff0ff0ff; break; //mvendorid
                    case 0x301: pre_rd = 0x40401101; break; //misa (XLEN=32, IMA+X)
                    default:
                    {
                        pre_rd = iv_uart.HandleOtherCSRRead( (uint8_t*) RESET_VECTOR, csrno );
                        break;
                    }   
                }

                switch( microop )
                {
                    case 1: writeval = rs1; break;  			//CSRRW
                    case 2: writeval = pre_rd | rs1; break;		//CSRRS
                    case 3: writeval = pre_rd & ~rs1; break;    //CSRRC
                    case 5: writeval = rs1imm; break;			//CSRRWI
                    case 6: writeval = pre_rd | rs1imm; break;	//CSRRSI
                    case 7: writeval = pre_rd & ~rs1imm; break;	//CSRRCI
                }
                switch( csrno )
                {
                    case 0x340: state.mscratch = writeval; break;
                    case 0x305: 
                        state.mtvec = writeval; 
                        break;
                    case 0x304: state.mie = writeval; break;
                    case 0x344: state.mip = writeval; break;
                    case 0x341: 
                        state.mepc = writeval; 
                        // printf("set mepc \n");
                        break;
                    case 0x300: state.mstatus = writeval; break; //mstatus
                    case 0x342: state.mcause = writeval; break;
                    case 0x343: state.mtval =  writeval; break;
                    default:
                    {
                        iv_uart.HandleOtherCSRWrite( (uint8_t*) RESET_VECTOR, csrno, writeval );
                        break;
                    }

                }
            }
            else if( microop == 0x0 ) // "SYSTEM" 0b000 ECALL/EBREAK
            {
                // printf("SYSTEM, csrno:%08x\n", csrno);
                rd_idx = 0;
                if( csrno == 0x105 ) //WFI (Wait for interrupts)
                {
                    // printf("\nWFI\n");
                    // printf("mstatus:%08x, extraflags:%08x\n", state.mstatus, state.extraflags);
                    state.mstatus |= 8;    //Enable interrupts
                    state.extraflags |= 4; //Infor environment we want to go to sleep.
                    // SETCSR( pc, (pc + 4) );
                    state.pc += 4;
                    // return 1;
                    ret_val = 1;
                }
                else if( ( ( csrno & 0xff ) == 0x02 ) )  // MRET
                {
                    // printf("MRET csrno:%08x/n", csrno);
                    //https://raw.githubusercontent.com/riscv/virtual-memory/main/specs/663-Svpbmt.pdf
                    //Table 7.6. MRET then in mstatus/mstatush sets MPV=0, MPP=0, MIE=MPIE, and MPIE=1. La
                    // Should also update mstatus to reflect correct mode.
                    uint32_t startmstatus = state.mstatus;
                    uint32_t startextraflags = state. extraflags;
                    // SETCSR( mstatus , (( startmstatus & 0x80) >> 4) | ((startextraflags&3) << 11) | 0x80 );
                    state.mstatus = (( startmstatus & 0x80) >> 4) | ((startextraflags&3) << 11) | 0x80;

                    // SETCSR( extraflags, (startextraflags & ~3) | ((startmstatus >> 11) & 3) );
                    state.extraflags = (startextraflags & ~3) | ((startmstatus >> 11) & 3);
                    state.pc = state.mepc - 4;
                    // printf("\n pc:%08x, line:%d\n",state.pc, __LINE__);
                }
                else
                {
                    // printf("else branch csrno:%08x\n", csrno);
                    switch( csrno )
                    {
                    case 0: trap = (state.extraflags & 3) ? (11+1) : (8+1); break; // ECALL; 8 = "Environment call from U-mode"; 11 = "Environment call from M-mode"
                    case 1:	trap = (3+1); break; // EBREAK 3 = "Breakpoint"
                    case 3: 
                        // printf("this is for func return \n");
                        ret_val = 3;
                        break;
                    default: trap = (2+1); break; // Illegal opcode.
                    }
                }
            }
            else
                trap = (2+1); 				// Note micrrop 0b100 == undefined.
            break;
        } 

        case 0x2f: // RV32A (0b00101111)
        {
            // printf("rv32a\n");
            uint32_t rs1 = state.gpr[((ir >> 15) & 0x1f)].val;
            uint32_t rs2 = state.gpr[((ir >> 20) & 0x1f)].val;
            uint32_t irmid = ( ir>>27 ) & 0x1f;


            // We don't implement load/store from UART or CLNT with RV32A here.

            if( rs1 >= MEM_RANGE-3 )
            {
                trap = (7+1); //Store/AMO access fault
                pre_rd = rs1;
            }
            else
            {
                pre_rd = iv_mem.load( rs1 ,4, false);

                // Referenced a little bit of https://github.com/franzflasch/riscv_em/blob/master/src/core/core.c
                uint32_t dowrite = 1;
                switch( irmid )
                {
                    case 2: //LR.W (0b00010)
                        dowrite = 0;
                        state.extraflags = (state.extraflags & 0x07) | (rs1<<3);
                        break;
                    case 3:  //SC.W (0b00011) (Make sure we have a slot, and, it's valid)
                        pre_rd = ( state.extraflags  >> 3 != ( rs1 & 0x1fffffff ) );  // Validate that our reservation slot is OK.
                        dowrite = !pre_rd; // Only write if slot is valid.
                        break;
                    case 1: break; //AMOSWAP.W (0b00001)
                    case 0: rs2 += pre_rd; break; //AMOADD.W (0b00000)
                    case 4: rs2 ^= pre_rd; break; //AMOXOR.W (0b00100)
                    case 12: rs2 &= pre_rd; break; //AMOAND.W (0b01100)
                    case 8: rs2 |= pre_rd; break; //AMOOR.W (0b01000)
                    case 16: rs2 = ((int32_t)rs2<(int32_t)pre_rd)?rs2:pre_rd; break; //AMOMIN.W (0b10000)
                    case 20: rs2 = ((int32_t)rs2>(int32_t)pre_rd)?rs2:pre_rd; break; //AMOMAX.W (0b10100)
                    case 24: rs2 = (rs2<pre_rd)?rs2:pre_rd; break; //AMOMINU.W (0b11000)
                    case 28: rs2 = (rs2>pre_rd)?rs2:pre_rd; break; //AMOMAXU.W (0b11100)
                    default: trap = (2+1); dowrite = 0; break; //Not supported.
                }
                if( dowrite ) iv_mem.store( rs1, rs2 ,4);
            }
            break;
        }
        default: trap = (2+1); // Fault: Invalid opcode.
    }//switch
}