# tiny-rv32sim

## what & why?
A toy when reading RV ISA, it is a "simple" "simulator" for learning porpuse, so there must be some bugs in it :)

## how?
 * git clone
 * make
### run kernel
 * ./riscv-simulator -i Image
 * inpur "r" to run
### run other program
 * cd tests
 * make
 * ./riscv-simulator -i tests/build/test.elf

## others but important
This "simulator" is only capable of "boot" the Linux kernel; you cannot use it in the same way as other simulators(such as qume).
The project is for learning purposes only and not intended for open source, so some partially implemented useless code ~~may~~ must be present in it. There may be future updates :)
