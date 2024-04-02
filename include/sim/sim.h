#ifndef __SIM_H
#define __SIM_H

#include "../memory/mem.h"
#include "../device/uart.h"
#include <string>
#include <map>
#include <assert.h>
#include <vector>
#include <core/cpu.h>

//forward define class to update file name
class option_parser_t;

class sim_t{
private:
    uint32_t entry_point;
    std::map<std::string, uint32_t> symbols;
    const uint32_t img[1] = {
        0x12000513,
    };
    void load_payload(const std::string &payload, mmu_t& iv_mem);
    
public:
    sim_t(){}
    char* file = nullptr;

    void run(cpu_t& cpu, mmu_t& iv_mem, uart& iv_uart, decode& iv_decode, riscv32_cpu_state& state);
    void reset();
    void load_img(mmu_t& iv_mem);
    std::map<std::string, uint32_t> load_elf(mmu_t& iv_mem, const char* file, riscv32_cpu_state& state);
    char* get_filename();
    void win_run(cpu_t& cpu, mmu_t& iv_mem, riscv32_cpu_state& state, uart& iv_uart, decode& iv_decode);
    void update_dtb(mmu_t& iv_mem, riscv32_cpu_state& state);
    friend class option_paser_t;
};

#endif