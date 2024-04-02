#include "core/instruction_list.h"
#if 0
uint32_t func_addi(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    cpu.gpr[inst.i.rd].val = cpu.gpr[inst.i.rs1].val + inst.i.simm11_0;
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
    return 0;
}

uint32_t func_add(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    cpu.gpr[inst.i.rd].val = cpu.gpr[inst.i.rs1].val + inst.i.simm11_0;
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
    return 0;
}

uint32_t func_jal(riscv32_cpu_state& cpu, instr inst){
    printf(">%s, val:%08x, opcode:%08x\n", __FUNCTION__, inst.val, ((inst.j.opcode6_2<<2)|(inst.j.opcode1_0)));
    uint32_t imm = ((inst.j.imm20<<20) | (inst.j.imm19_12<<12) | (inst.j.imm11<<11) | (inst.j.imm10_1<<1));
    if(inst.j.imm20 == 1){
        imm = imm | 0xffe00000;//signed 
    }
    // printf("IMM:%08x\n", imm);
    cpu.pc = cpu.pc + imm;
    cpu.pc -= 4;//at last step will + 4, so sub 4 for correct value 
}

uint32_t func_lui(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    cpu.gpr[inst.u.rd].val = inst.u.imm31_12;
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}
#include "memory/mem.h"
uint32_t func_lb(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.i.simm11_0;
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    cpu.gpr[inst.i.rd].val = mem.read(inst.i.rs1 + imm, 1);
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_lh(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.i.simm11_0;
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    cpu.gpr[inst.i.rd].val = mem.read(inst.i.rs1 + imm, 2);
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_lw(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.i.simm11_0;
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    cpu.gpr[inst.i.rd].val = mem.read(inst.i.rs1 + imm, 4);
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_lbu(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.i.simm11_0;
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    cpu.gpr[inst.i.rd].val = mem.read(inst.i.rs1 + imm, 1);
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_lhu(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.i.simm11_0;
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    cpu.gpr[inst.i.rd].val = mem.read(inst.i.rs1 + imm, 2);
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_sb(riscv32_cpu_state& cpu, instr inst){
    //rs1 + imm -> rs2
    printf(">%s\n", __FUNCTION__);
    mmu_t mem;
    uint32_t imm = inst.s.imm4_0 | (inst.s.simm11_5<<5);
    imm = imm | ((imm & 0x800)?0xfffff000:0);
    mem.write(cpu.gpr[inst.s.rs1].val + imm, cpu.gpr[inst.s.rs2].val);
}

uint32_t func_auipc(riscv32_cpu_state& cpu, instr inst){
    printf(">%s\n", __FUNCTION__);
    cpu.gpr[inst.u.rd].val = cpu.pc + inst.u.imm31_12;
    if(inst.i.rd == 0) cpu.gpr[inst.i.rd].val = 0;
}

uint32_t func_beq(riscv32_cpu_state& cpu, instr inst){
    if(cpu.gpr[inst.b.rs1].val == cpu.gpr[inst.b.rs2].val) cpu.pc += ((inst.b.imm11<<11)|(inst.b.imm4_1<<1)|(inst.b.imm10_5<<5)|(inst.b.imm12<<12));
    cpu.pc -= 4;
}

uint32_t func_bne(riscv32_cpu_state& cpu, instr inst){
    if(cpu.gpr[inst.b.rs1].val != cpu.gpr[inst.b.rs2].val) cpu.pc += ((inst.b.imm11<<11)|(inst.b.imm4_1<<1)|(inst.b.imm10_5<<5)|(inst.b.imm12<<12));
    cpu.pc -= 4;
}

uint32_t func_blt(riscv32_cpu_state& cpu, instr inst){
    if(cpu.gpr[inst.b.rs1].val < cpu.gpr[inst.b.rs2].val) cpu.pc += ((inst.b.imm11<<11)|(inst.b.imm4_1<<1)|(inst.b.imm10_5<<5)|(inst.b.imm12<<12));
    cpu.pc -= 4;
}

uint32_t func_bge(riscv32_cpu_state& cpu, instr inst){
    if(cpu.gpr[inst.b.rs1].val >= cpu.gpr[inst.b.rs2].val) cpu.pc += ((inst.b.imm11<<11)|(inst.b.imm4_1<<1)|(inst.b.imm10_5<<5)|(inst.b.imm12<<12));
    cpu.pc -= 4;
}

#endif