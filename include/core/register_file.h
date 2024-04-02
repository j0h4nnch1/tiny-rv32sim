#ifndef __REG_H
#define __REG_H

#include <cstdint>
struct reg_t{
    uint32_t val;
};

typedef struct {
  struct {
    uint32_t val;
  } gpr[32];

  uint32_t pc;

  uint32_t timerl;
  uint32_t timerh;
  uint32_t timermatchh;
  uint32_t timermatchl;
  uint32_t mscratch;
  uint32_t mtvec;
  uint32_t mie;
  uint32_t mip;
  uint32_t mepc;
  uint32_t mstatus;
  uint32_t mcause;
  uint32_t mtval;
  /**
   * User: 00
   * Supervisor: 01
   * Machine: 11
  */
  uint32_t extraflags;
} riscv32_cpu_state;
#endif