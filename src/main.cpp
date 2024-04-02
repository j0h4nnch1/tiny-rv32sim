#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "common/option_parser.h"
#include "sim/sim.h"

int main(int argc, char** argv){
    sim_t sim; 
    option_parser_t parser;
    cpu_t cpu;
    mmu_t iv_mem;
    riscv32_cpu_state state = {};
    uart iv_uart(state);
    decode iv_decode;
    parser.add_option('h', "help", [&](const char UNUSED *s){parser.help();});
    parser.add_option('i', "img", [&](const char UNUSED *s){parser.update_img(sim, s);});
    // parser.add_option('f', "file", [&](const char UNUSED *s){parser.update_img(sim, s);});
    parser.parse(argv);
    sim.run(cpu, iv_mem, iv_uart, iv_decode, state);

    return 0;
}