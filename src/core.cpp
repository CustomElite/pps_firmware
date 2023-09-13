#include "core.h"

static volatile uint32_t s_Milli = 0;

uint32_t GetMilli(void)
{
    return s_Milli;
}

void ErrorHandler(const char* err_str)
{
    Serial::Printf("Critical Error: %s\n", err_str);

    __disable_irq();
    while (1)
    {

    }
}

extern "C" void SysTick_Handler(void)
{
    s_Milli += 1u;
}