#include "core.h"
#include "peripherals.h"
#include "external/dac80004.hpp"
#include "common/containers/FIFO.hpp"

#include <string>

using dac_t = External::DAC80004::DAC80004;
using DAC_Channel = External::DAC80004::Channel;
//Containers::FIFO < char, 64, true > SerialBuffer;

adc_t ADC(SPI2, ADC_CS_PORT, ADC_CS_PIN, DMA1, ADC_RX_DMA_CHANNEL, ADC_TX_DMA_CHANNEL);
dac_t DAC(SPI1, DAC_SYNC_PORT, DAC_SYNC_PIN);

int main(void)
{
    /* System interrupt init*/
    __NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    //LL_GPIO_AF_Remap_SWJ_NOJTAG();

    /* Initialize all configured peripherals */
    SysClock_Config();
    GPIO_Init();
    SPI1_Init();
    SPI2_Init();
    DMA1_Init();

    ADC.Enable();
    DAC.Enable();

    Serial::Init();
    Serial::Print("Hello World!!!\n");

    uint32_t timer = 0;
    uint16_t dacD = 0;
    DAC.SetPowerState(External::DAC80004::PowerState::Off_1k, true, true, true, false);

    std::string SerialRx;
    
    /* Infinite loop */
    while (1)
    {
        if ((GetMilli() - timer) >= 1000U)
        {
            LL_GPIO_TogglePin(STATUS_LED_PORT, STATUS_LED_PIN);
            timer = GetMilli();
            DAC.SetAndUpdate(DAC_Channel::D, dacD);
            //Serial::Printf("Millisec: %d\n", timer);
            //Serial::Printf("Dac D: %d\n", dacD);
            dacD = (dacD > 65535u) ? 0 : (dacD + 100);

            if (ADC.ConversionReady())
            {
                int32_t conv = ADC.GetConversion();
                Serial::Printf("ADC Code: %d | Voltage: %0.6fv\n", conv, ADC.ToVoltage(conv));
            }
        }
        
        while (Serial::Available())
        {
            char rxChar = Serial::Read();

            if (rxChar == ';')
            {
                Serial::Print(SerialRx);
                SerialRx.clear();
            }
            else {
                SerialRx += rxChar;
            }
        }

        
    }
}

