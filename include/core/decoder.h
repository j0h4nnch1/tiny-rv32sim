#ifndef __DECODER_H
#define __DECODER_H
#include <cstdint>
#include <string>
#include "register_file.h"
#include "memory/mem.h"
#include "../device/uart.h"
// #include "instruction.h"
// #include "cpu.h" //todo: deocoder should not include cpu...

union instr{
    struct {
        uint32_t opcode1_0 : 2;
        uint32_t opcode6_2 : 5;
        uint32_t rd        : 5;
        uint32_t funct3    : 3;
        uint32_t rs1       : 5;
        int32_t  simm11_0  :12;
    } i;
    struct {
        uint32_t opcode1_0 : 2;
        uint32_t opcode6_2 : 5;
        uint32_t imm4_0    : 5;
        uint32_t funct3    : 3;
        uint32_t rs1       : 5;
        uint32_t rs2       : 5;
        int32_t  simm11_5  : 7;
    } s;
    struct {
        uint32_t opcode1_0 : 2;
        uint32_t opcode6_2 : 5;
        uint32_t rd        : 5;
        uint32_t imm31_12  :20;
    } u;
    struct {
        uint32_t opcode1_0 : 2;
        uint32_t opcode6_2 : 5;
        uint32_t rd        : 5;
        uint32_t imm19_12  : 8;
        uint32_t imm11     : 1;
        uint32_t imm10_1   : 10;
        uint32_t imm20     : 1;
    } j;
    struct {
        uint32_t opcode1_0 : 2;
        uint32_t opcode6_2 : 5;
        uint32_t imm11     : 1;
        uint32_t imm4_1    : 4;
        uint32_t funct3    : 3;
        uint32_t rs1       : 5;
        uint32_t rs2       : 5;
        uint32_t imm10_5   : 6;
        uint32_t imm12     : 1;
    } b;

    uint32_t val;
};

class cpu_t;

struct decoder
{
    uint32_t _mask;
    uint32_t _match;
    uint32_t (*func)(riscv32_cpu_state&, instr);
};

class decode{
    public:
    void decode_run_without_trap(uint32_t ir, uint32_t& trap, uint32_t& pre_rd, uint32_t& rd_idx, riscv32_cpu_state& state, int32_t& ret_val, uart& iv_uart, mmu_t& iv_mem);
};


#endif