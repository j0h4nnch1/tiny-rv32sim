.section entry, "ax"
.globl _start
.type _start, @function
# .equ UART_BASE, 0x10000000
# .globl _turing_machine
_start:
  mv s0, zero
  la sp, _stack_pointer
  li t0, 0
  csrs mtvec, t0
  csrs mepc, t0
  sb a0, 0(a1)
  jal _turing_machine

.globl _halt
_halt:
  .word 0x00300073



