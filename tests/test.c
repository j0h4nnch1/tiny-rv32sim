#include "uart.h"

int main() {
    print("hello\n");
    int sum = 0;
    for(int i = 0; i < 100; i++){
        sum += i;
    }
    print("end\n");
    return 0;
}