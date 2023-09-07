#include "core.h"

// Interrupt service routine
extern "C" 
{
    void EXTI15_10_IRQHandler(void)
    {
        if (LL_EXTI_IsActiveFlag_0_31(ADC_BSY_EXTI_LINE))
        {
            LL_EXTI_ClearFlag_0_31(ADC_BSY_EXTI_LINE);
            ADC.EoC_Callback();
        }
    }

    void DMA1_Channel4_IRQHandler(void)
    {
        ADC.DMA_Callback();
    }

    void DMA1_Channel5_IRQHandler(void)
    {
    }
}
