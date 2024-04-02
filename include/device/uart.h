#ifndef _UART_H
#define _UART_H
#define SERIAL_BASE_ADDRESS 0x10000000

#include <cstdint>
#include "../memory/mem.h"
#include "../core/register_file.h"
#if 0
void write_char_to_serial(char c) {
    volatile char* serial = (volatile char*)SERIAL_BASE_ADDRESS;
    *serial = c;
}

void print(const char* str){
    for(const char* a= str; *a!='\0'; a++){
        write_char_to_serial(*a);
    }
}
#endif
class uart{
public:
    uint32_t HandleControlLoad(uint32_t addy);
    uint32_t HandleControlStore(uint32_t addy, uint32_t val);
    int32_t HandleOtherCSRRead(uint8_t * image, uint16_t csrno);
    void HandleOtherCSRWrite(uint8_t * image, uint16_t csrno, uint32_t value);
    int IsKBHit();
    int ReadKBByte();
    uart(riscv32_cpu_state& state){
        is_eofd = 0;
        state.gpr[11].val = 0x83fff940;
    }
    ~uart(){}
private:
    int is_eofd;
};


#endif