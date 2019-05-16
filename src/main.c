// main.c

#include "Master.h"

int colorIndex = 0;

void HardFault_Handler(void) {
    while(1) {
        for(int i = 0; i < 800000; ++i);
        colorIndex = (colorIndex + 1) % 0x08;
        LED = colorIndex << 1;
    }
}

int main(void){
    PLL_Init(Bus80MHz);
    Output_Init();
    STDIOMain();
}
