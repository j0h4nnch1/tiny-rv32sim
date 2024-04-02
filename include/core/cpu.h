#ifndef __CPU_H
#define __CPU_H

#include <cstdint>
#include "register_file.h"
#include "decoder.h"
#include "encoding.h"
#include "memory/mem.h"
#include <unordered_map>
#include <vector>

#include <stdint.h>
#include <string.h>

#include <chrono>
#include <thread>
#include "dtb.h"

typedef uint64_t pc_t;
class cpu_t{
public:
    uint32_t decode_run(riscv32_cpu_state& state, instr data, mmu_t& iv_mem, uint32_t time_sed, uart& iv_uart, decode& iv_decode);
    pc_t fetch_decode_exec(mmu_t& iv_mem, riscv32_cpu_state& cpu, uart& iv_uart, decode& iv_decode);
    void register_func(decoder decode);
    void init(riscv32_cpu_state& cpu);
	uint32_t handle_exception(riscv32_cpu_state& state, uint32_t ir, uint32_t trap);
	void dump_state(riscv32_cpu_state& state, instr data);
	uint64_t GetTimeMicroseconds();
private:
//string, handle
    std::vector<decoder> all_inst;

    std::unordered_map<std::string, decoder> all_func;

};

#endif