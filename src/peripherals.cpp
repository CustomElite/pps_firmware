#include "core.h"
#include "peripherals.h"

void SysClock_Config(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0) {}

    LL_RCC_HSE_Enable();
    while(!LL_RCC_HSE_IsReady()) {}

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE) {}
    
    LL_Init1msTick(8000000);
    LL_SetSystemCoreClock(8000000);

    __NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(__NVIC_GetPriorityGrouping(),15, 0));
    __NVIC_EnableIRQ(SysTick_IRQn);

    LL_SYSTICK_EnableIT();
}

void GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB | LL_APB2_GRP1_PERIPH_GPIOC);

    LL_GPIO_SetOutputPin(STATUS_LED_PORT, STATUS_LED_PIN);
    LL_GPIO_SetOutputPin(ADC_CS_PORT, ADC_CS_PIN);
    
    /* Status LED Pin */
    LL_GPIO_SetPinMode(STATUS_LED_PORT, STATUS_LED_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(STATUS_LED_PORT, STATUS_LED_PIN, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(STATUS_LED_PORT, STATUS_LED_PIN, LL_GPIO_OUTPUT_PUSHPULL);

    /* ADC Chip Select Pin */
    LL_GPIO_SetPinMode(ADC_CS_PORT, ADC_CS_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(ADC_CS_PORT, ADC_CS_PIN, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(ADC_CS_PORT, ADC_CS_PIN, LL_GPIO_OUTPUT_PUSHPULL);

    /* ADC Busy Pin: External Interrupt /w Falling Edge Trigger */
    LL_GPIO_SetPinMode(ADC_BSY_PORT, ADC_BSY_PIN, LL_GPIO_MODE_FLOATING);
    LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE11);

    __NVIC_SetPriority(ADC_BSY_EXTI_IRQn, NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), 0, 0));
    __NVIC_EnableIRQ(ADC_BSY_EXTI_IRQn);

    LL_EXTI_EnableIT_0_31(ADC_BSY_EXTI_LINE);
    LL_EXTI_EnableFallingTrig_0_31(ADC_BSY_EXTI_LINE);

}

void DMA1_Init()
{
    LL_DMA_InitTypeDef dmaInit = {0};

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    __NVIC_SetPriority(ADC_RX_DMA_IRQn, NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), 0, 0));
    __NVIC_EnableIRQ(ADC_RX_DMA_IRQn);

    __NVIC_SetPriority(ADC_TX_DMA_IRQn, NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), 0, 0));
    __NVIC_EnableIRQ(ADC_TX_DMA_IRQn);

    dmaInit.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    dmaInit.Mode = LL_DMA_MODE_NORMAL;
    dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
    dmaInit.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
    dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    LL_DMA_Init(DMA1, ADC_RX_DMA_CHANNEL, &dmaInit);

    dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    LL_DMA_Init(DMA1, ADC_TX_DMA_CHANNEL, &dmaInit);
}

void SPI2_Init()
{
    LL_SPI_InitTypeDef spiConfig = {0};
    LL_GPIO_InitTypeDef gpioConfig = {0};
        
    /* GPIO Initialization */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);

    /* Sclk & MOSI */
    gpioConfig.Pin = LL_GPIO_PIN_13 | LL_GPIO_PIN_15;
    gpioConfig.Mode = LL_GPIO_MODE_ALTERNATE;
    gpioConfig.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpioConfig.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOB, &gpioConfig);
    /* MISO */
    gpioConfig.Pin = LL_GPIO_PIN_14;
    gpioConfig.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOB, &gpioConfig);

    /* Peripheral Initialization*/
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

    spiConfig.TransferDirection = LL_SPI_FULL_DUPLEX;
    spiConfig.Mode = LL_SPI_MODE_MASTER;
    spiConfig.DataWidth = LL_SPI_DATAWIDTH_16BIT;
    spiConfig.ClockPolarity = LL_SPI_POLARITY_LOW;
    spiConfig.ClockPhase = LL_SPI_PHASE_1EDGE;
    spiConfig.NSS = LL_SPI_NSS_SOFT;
    spiConfig.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
    spiConfig.BitOrder = LL_SPI_MSB_FIRST;
    spiConfig.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    spiConfig.CRCPoly = 10;
    LL_SPI_Init(SPI2, &spiConfig);

    LL_SPI_EnableDMAReq_RX(SPI2);
    LL_SPI_EnableDMAReq_TX(SPI2);
}