extern "C" {
#include "main.h"
}

#include "stm32l4xx.h"

extern "C" void Error_Handler(void) {
    __disable_irq();

    while (true) {
        __NOP();
    }
}