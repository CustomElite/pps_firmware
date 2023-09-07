#include "serial.h"
#include "printf.h"
#include "common/containers/FIFO.hpp"

//static void usart_write(uint8_t c);

constexpr static uint8_t RxBufferSize = 64u;

using buffer_t = Containers::FIFO<uint8_t, RxBufferSize, true>;

static buffer_t s_RxBuffer;

static inline void usart_write(uint8_t c)
{
    while(!LL_USART_IsActiveFlag_TXE(USART1)) {}
	LL_USART_TransmitData8(USART1, c);
}

namespace Serial 
{
    void Init()
    {
        /* Peripheral clock enable */
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA | LL_APB2_GRP1_PERIPH_USART1);
        
        /* USART1 GPIO Configuration */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
        LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL);
    
        LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_10, LL_GPIO_PULL_UP);
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_FLOATING);
        LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);
        
        /* UART Configuration */
        LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);
        LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
        LL_USART_SetBaudRate(USART1, __LL_RCC_CALC_PCLK2_FREQ(SystemCoreClock, LL_RCC_GetAPB2Prescaler()), 115200U);
        LL_USART_ConfigAsyncMode(USART1);
    
        __NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(__NVIC_GetPriorityGrouping(), 10, 0));
        __NVIC_EnableIRQ(USART1_IRQn);
        LL_USART_EnableIT_RXNE(USART1);
    
        LL_USART_Enable(USART1);
    }
    
    char Read()
    {
        if (s_RxBuffer.Empty()) return 0;
    
        uint8_t c = s_RxBuffer.Pop();
    
        return (char)c;
    }
    
    int Print(const std::string& str)
    {
        for (auto &&c : str)
            usart_write(c);
    
        return str.size();
    }
    
    int Printf(const char* format, ...)
    {
        std::va_list arg;
        va_start(arg, format);
    
        const int i = vprintf_(format, arg);
    
        va_end(arg);
        return i;
    }
    
    uint8_t Available()
    {
        return s_RxBuffer.Size();
    }
} // namespace Serial

// Required for printf functionality
void putchar_(char ch)
{
    usart_write(ch);
}

extern "C" void USART1_IRQHandler(void)
{
    if (LL_USART_IsActiveFlag_RXNE(USART1))
        s_RxBuffer.Push(LL_USART_ReceiveData8(USART1));
}