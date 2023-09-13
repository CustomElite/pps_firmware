#ifndef _CORE_H_
#define _CORE_H_

#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"

#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_spi.h"

#include "serial/serial.h"
#include "external/ltc2440.hpp"

#include <cstdint>

#ifndef NVIC_PRIORITYGROUP_0
  #define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007)
  #define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006)
  #define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005)
  #define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004)
  #define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003)
#endif

#define STATUS_LED_PIN 	    LL_GPIO_PIN_13
#define STATUS_LED_PORT 	  GPIOC

#define ADC_RX_DMA_CHANNEL  LL_DMA_CHANNEL_4
#define ADC_RX_DMA_IRQn     DMA1_Channel4_IRQn
#define ADC_TX_DMA_CHANNEL  LL_DMA_CHANNEL_5
#define ADC_TX_DMA_IRQn     DMA1_Channel5_IRQn

#define ADC_CS_PIN          LL_GPIO_PIN_12
#define ADC_CS_PORT         GPIOB

#define ADC_BSY_PIN         LL_GPIO_PIN_11
#define ADC_BSY_PORT        GPIOB

#define ADC_BSY_EXTI_LINE   LL_EXTI_LINE_11
#define ADC_BSY_EXTI_IRQn   EXTI15_10_IRQn

#define DAC_SYNC_PIN        LL_GPIO_PIN_10
#define DAC_SYNC_PORT       GPIOB

using adc_t = External::LTC2440::LTC2440;

#ifdef __cplusplus
extern "C" {
#endif

  uint32_t GetMilli();
  void ErrorHandler(const char* err_str);

#ifdef __cplusplus
}
#endif

extern adc_t ADC;

#endif