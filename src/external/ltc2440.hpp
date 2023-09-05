#pragma once

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"

#include "common/containers/FIFO.hpp"
#include <utility>
#include <cstddef>

namespace External::LTC2440
{
    enum OverSampling : uint8_t
    {
        OSR_64      = (0b0001 << 3u),
        OSR_128     = (0b0010 << 3u),
        OSR_256     = (0b0011 << 3u),
        OSR_512     = (0b0100 << 3u),
        OSR_1024    = (0b0101 << 3u),
        OSR_2048    = (0b0110 << 3u),
        OSR_4096    = (0b0111 << 3u),
        OSR_8192    = (0b1000 << 3u),
        OSR_16384   = (0b1001 << 3u),
        OSR_32768   = (0b1111 << 3u)
    };

    struct Properties
    {
        SPI_TypeDef* SPI;
        GPIO_TypeDef* CS_Port;
        uint32_t CS_Pin;
        DMA_TypeDef* DMA;
        uint32_t RxChannel, TxChannel;
        OverSampling OSR;

        Properties() noexcept = default;
        Properties(SPI_TypeDef* spi, 
                   GPIO_TypeDef* cs_port, 
                   uint32_t cs_pin,
                   DMA_TypeDef* dma,
                   uint32_t rx_channel, 
                   uint32_t tx_channel,
                   OverSampling osr = OSR_32768) noexcept
        : SPI(spi),
        CS_Port(cs_port),
        CS_Pin(cs_pin),
        DMA(dma), 
        RxChannel(rx_channel), 
        TxChannel(tx_channel)
        {}
    };

    class LTC2440 : Properties
    {
    public:
        using buffer_t = Containers::FIFO<int32_t, 4u>;

        template <typename... Args>
        LTC2440(Args&&... args) : Properties{ std::forward<Args>(args)... }
        {
            m_TX.byte[3] = OSR;
        }
        void Enable() noexcept
        {
            LL_SPI_Enable(SPI);
        }
        void Disable() noexcept
        {
            LL_SPI_Disable(SPI);
        }
        void EoC_Callback() noexcept
        {
            spi_read_blocking();
        }
        bool ConversionReady() const { return (!m_conversions.Empty()); }
        bool ClearConversion() noexcept
        {
            LL_GPIO_ResetOutputPin(CS_Port, CS_Pin);     
            LL_mDelay( 1U );
    
            LL_SPI_TransmitData8(SPI, m_TX.byte[3]);
            while(!LL_SPI_IsActiveFlag_TXE(SPI));
    
            while (!LL_SPI_IsActiveFlag_RXNE(SPI));
            LL_SPI_ReceiveData8(SPI);
    
            LL_GPIO_SetOutputPin(CS_Port, CS_Pin);
    
            return true;
        }
        int32_t GetConversion() noexcept
        {
            if (m_conversions.Empty()) return 0;

            int32_t output = m_conversions.Pop();
            output -= BIN_OFFSET;
            
            return output;
        }

    private:
        constexpr static const float V_REF = 4.096f;
        constexpr static const uint32_t MAX_CODE = 0xFFFFFF;
        constexpr static const uint32_t BIN_OFFSET = 0x10000000;

    private:
        buffer_t m_conversions;
        OverSampling m_OSR;

        union {
            uint8_t byte[4];
            uint16_t word[2];
            int32_t dword;
        } m_TX, m_RX;

    private:
        void spi_read_blocking() noexcept
        {
            LL_GPIO_ResetOutputPin(CS_Port, CS_Pin);
            LL_mDelay( 1U );
            for (int8_t i = 1; i >= 0; --i)
            {
                LL_SPI_TransmitData16(SPI, m_TX.word[i]);
                while(!LL_SPI_IsActiveFlag_TXE(SPI));
        
                while (!LL_SPI_IsActiveFlag_RXNE(SPI));
                m_RX.word[i] = LL_SPI_ReceiveData16(SPI);
            }
            LL_GPIO_SetOutputPin(CS_Port, CS_Pin);

            m_conversions.Push(m_RX.dword);
        }
    };
}
