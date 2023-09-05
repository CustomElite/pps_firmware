#include "core.h"
#include "peripherals.h"
#include "serial/serial.h"

adc_t ADC(SPI2, ADC_CS_PORT, ADC_CS_PIN, DMA1, ADC_RX_DMA_CHANNEL, ADC_TX_DMA_CHANNEL);

int main(void)
{
    /* System interrupt init*/
    __NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    //LL_GPIO_AF_Remap_SWJ_NOJTAG();
    
    /* Initialize all configured peripherals */
    SysClock_Config();
    GPIO_Init();
    SPI2_Init();
    ADC.Enable();
    Serial::Init();
    Serial::Print("Hello World!!!\n");

    uint32_t timer = 0;
    uint32_t average = 0;
    size_t avgIdx = 0;
    
    /* Infinite loop */
    while (1)
    {
        if ((GetMilli() - timer) >= 1000U)
        {
            LL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);

            while (ADC.ConversionReady())
            {
                average += ADC.GetConversion();
                ++avgIdx;
            }

            average /= avgIdx;
            Serial::Printf("ADC Average: %d | # Reads: %d\n", average, avgIdx);
            average = 0;
            avgIdx = 0;
            timer = GetMilli();
        }
        
        if (Serial::Available())
        {
        }
    }
}

