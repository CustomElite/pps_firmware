#include "core.h"
#include "peripherals.h"

adc_t ADC(SPI2, ADC_CS_PORT, ADC_CS_PIN, DMA1, ADC_RX_DMA_CHANNEL, ADC_TX_DMA_CHANNEL);

int main(void)
{
    /* System interrupt init*/
    __NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    //LL_GPIO_AF_Remap_SWJ_NOJTAG();
    STAT_LED statusLED(IO::Mode::Output, IO::Type::PushPull, IO::Speed::Low);
    statusLED = IO::State::Low;
    
    /* Initialize all configured peripherals */
    SysClock_Config();
    GPIO_Init();
    SPI1_Init();
    SPI2_Init();
    DMA1_Init();

    ADC.Enable();

    Serial::Init();
    Serial::Print("Hello World!!!\n");

    uint32_t timer = 0;
    bool state = false;
    
    /* Infinite loop */
    while (1)
    {
        if ((GetMilli() - timer) >= 1000U)
        {
            statusLED = state;
            state = (state) ? false : true;
            timer = GetMilli();
        }
        
        if (Serial::Available())
        {
            Serial::Printf("%c", Serial::Read());
        }

        if (ADC.ConversionReady())
        {
            int32_t conv = ADC.GetConversion();
            Serial::Printf("ADC Code: %d | Voltage: %0.6fv\n", conv, ADC.ToVoltage(conv));
        }
    }
}

