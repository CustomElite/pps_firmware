#include "serial.h"
#include "printf.h"

//static void usart_write(uint8_t c);

static const uint8_t RxBufferSize = 64U;

static volatile uint8_t _rxBuffer[RxBufferSize] = {0};
static volatile uint8_t _rxBufHead = 0, _rxBufTail = 0;

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
        if (_rxBufTail == _rxBufHead) return 0;
    
        uint8_t c = _rxBuffer[_rxBufTail];
        _rxBufTail = (_rxBufTail + 1) % 64;
    
        return (char)c;
    }
    
    int Print(const std::string& str)
    {
        for (auto &c : str)
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
        return ((RxBufferSize + _rxBufHead - _rxBufTail) % RxBufferSize);
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
    {
        uint8_t i = (_rxBufHead + 1) % 64;
        if (i != _rxBufTail)
        {
            _rxBuffer[_rxBufHead] = LL_USART_ReceiveData8(USART1);
            _rxBufHead = i;
        }
    }
}