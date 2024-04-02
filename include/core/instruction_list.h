#ifndef __INST_H
#define __INST_H


#include "decoder.h"
#include "register_file.h"
#include "encoding.h"

uint32_t func_addi(riscv32_cpu_state cpu, instr inst);

#ifdef DECLARE_FUNC

DECLARE_FUNC(addi)
DECLARE_FUNC(lui)
DECLARE_FUNC(jal)


#endif
#endif