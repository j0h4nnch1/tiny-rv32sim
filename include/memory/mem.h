#ifndef __MEM_H
#define __MEM_H

#include <cstdint>
#include <cstdio>
#define VADDR_BASE 0x80000000
#define PADDR_BASE 0x80000000
#define RESET_VECTOR VADDR_BASE
#define RAM_SIZE 0x2000000
#define MEM_SIZE 0x4000000
#define MEM_RANGE (VADDR_BASE + MEM_SIZE)

class mmu_t{
public:
    mmu_t(){
        this->phymem = new uint8_t[MEM_SIZE];
    }
    uint32_t host2guest(uint8_t* addr);
    uint8_t* guest2host(uint32_t addr);
    uint32_t read_host_sign(void* addr, uint32_t len, bool sign);
    /**
     * addr: host addr
     * len : read len 1/2/4 byte
    */
    uint32_t read_host(void* addr, uint32_t len);
    /**
     * addr: host addr
     * 
    */
    template <typename T>
    void write_host(void* addr, const T& data);
    /**
     * g_addr: guest addr
    */
    uint32_t read(uint32_t g_addr, uint32_t len);
    void write(uint32_t g_addr, uint32_t len);
    uint32_t fetch(uint32_t pc);
    uint32_t load(uint32_t addr, uint32_t len, bool sign);
    uint32_t store(uint32_t addr, uint32_t data, uint32_t size);
    void init();
    void dump_memory(uint32_t start, uint32_t size);
    ~mmu_t(){
        delete phymem;
    }
    void get_phymem(){
        printf("%p - %p\n",phymem, phymem+MEM_SIZE);
    }

private:
    uint8_t* phymem;
};

#endif