#include "memory/mem.h"
#include <stdio.h>
#include <cassert>

uint32_t mmu_t::host2guest(uint8_t* addr){
    return (addr - this->phymem + PADDR_BASE);
}

uint8_t* mmu_t::guest2host(uint32_t addr){
    if(addr<PADDR_BASE||addr>MEM_RANGE){
        printf("out of range addr:%08x\n", addr);
        assert(1);
    }
    // assert(addr);
    return (uint8_t*)(this->phymem + addr - PADDR_BASE);
}

uint32_t mmu_t::read_host(void* addr, uint32_t len){
    switch (len){
    case 1:
        return *(uint8_t*)addr;
    case 2:
        return *(uint16_t*)addr;
    case 4:
        return *(uint32_t*)addr;
    default:
        printf("not support len\n");
        break;
    }
    return 0;
}

uint32_t mmu_t::read_host_sign(void* addr, uint32_t len, bool sign){
    switch (len){
    case 1:
        // printf("try to load 1 byte data from %08x\n", host2guest((uint8_t*)addr));
        return sign? *(int8_t*)addr:*(uint8_t*)addr;
    case 2:
        return sign? *(int16_t*)addr:*(uint16_t*)addr;
    case 4:
        // printf("try to load 4 byte data from %08x\n", host2guest((uint8_t*)addr));
        return *(uint32_t*)addr;
    default:
        printf("not support len\n");
        break;
    }
    return 0;
}

template <typename T>
void mmu_t::write_host(void* addr, const T& data){
    uint32_t size = sizeof(data);
    switch (size){
        case 1:
            *(uint8_t*)addr = data;
            break;
        case 2:
            *(uint16_t*)addr = data;
            break;
        case 4:
            *(uint32_t*)addr = data;
            break;
        default:
            break;
            printf("not support len\n");
    }
}
uint32_t mmu_t::read(uint32_t addr, uint32_t data){
    return read_host((void*)guest2host(addr),data);
}

void mmu_t::write(uint32_t addr, uint32_t data){
    return write_host((void*)guest2host(addr),data);
}

uint32_t mmu_t::load(uint32_t addr, uint32_t len, bool sign){
    return read_host_sign((void*)guest2host(addr), len, sign);
}

uint32_t mmu_t::store(uint32_t addr, uint32_t data, uint32_t size){
    
    switch (size){
        case 1:
            *guest2host(addr) = data;
            break;
        case 2:
            *(uint16_t*)guest2host(addr) = data;
            break;
        case 4:
            // printf("try to store a data\n");
            *(uint32_t*)guest2host(addr) = data;

            break;
        default:
            break;
            printf("not support len\n");
    }
    // return write_host((void*)guest2host(addr), data);
}

void mmu_t::init(){}

void mmu_t::dump_memory(uint32_t start, uint32_t size){
    // iv_mem.read_host((void*)iv_mem.guest2host(RESET_VECTOR + 4), 4);
    printf("----dump memory start----\n");
    for(int i = 0; i < size; ++i){
        uint32_t temp = read_host((void*)guest2host(start + 4 * i), 4);
        printf("0x%08x : %08x\n", start + 4 * i, temp);
    }
    printf("----dump memory end----\n");
}

uint32_t mmu_t::fetch(uint32_t pc){
    // printf(">%s, pc: %lx\n", __FUNCTION__, pc);
    uint32_t val = this->read(pc, 4);
    // printf("<%s\n", __FUNCTION__);
    return val;
}