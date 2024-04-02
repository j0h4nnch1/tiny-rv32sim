#define SERIAL_BASE_ADDRESS 0x10000000

void write_char_to_serial(char c) {
    volatile char* serial = (volatile char*)SERIAL_BASE_ADDRESS;
    *serial = c;
}

void print(const char* str){
    for(const char* a= str; *a!='\0'; a++){
        write_char_to_serial(*a);
    }
}