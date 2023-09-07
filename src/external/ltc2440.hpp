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
        TxChannel(tx_channel),
        OSR(osr)
        {}
    };

    class LTC2440 : Properties
    {
    public:
        using buffer_t = Containers::FIFO<int32_t, 4u>;

        template <typename... Args>
        LTC2440(Args&&... args) : Properties{ std::forward<Args>(args)... }
        {
            m_dmaTxBuffer[0] = (OSR << 8u);
        }
        void Enable() noexcept
        {
            LL_SPI_Enable(SPI);

            ClearConversion();

            LL_DMA_EnableIT_TC(DMA, RxChannel);
            LL_DMA_EnableIT_TE(DMA, RxChannel);

            LL_DMA_ConfigAddresses(DMA, RxChannel, LL_SPI_DMA_GetRegAddr(SPI), (uint32_t)m_dmaRxBuffer, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
            LL_DMA_ConfigAddresses(DMA, TxChannel, (uint32_t)m_dmaTxBuffer, LL_SPI_DMA_GetRegAddr(SPI), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        }
        void Disable() noexcept
        {
            LL_SPI_Disable(SPI);

            LL_DMA_DisableIT_TC(DMA, RxChannel);
            LL_DMA_DisableIT_TE(DMA, RxChannel);
        }
        void EoC_Callback() noexcept
        {
            dma_start_read();
        }
        void DMA_Callback() noexcept
        {
            dma_reset();
        }
        bool ConversionReady() const { return m_conversions.Full(); }
        bool ClearConversion() noexcept
        {
            LL_GPIO_ResetOutputPin(CS_Port, CS_Pin);     

            LL_SPI_TransmitData8(SPI, OSR);
            while(!LL_SPI_IsActiveFlag_TXE(SPI));

            while (!LL_SPI_IsActiveFlag_RXNE(SPI));
            LL_SPI_ReceiveData8(SPI);

            LL_GPIO_SetOutputPin(CS_Port, CS_Pin);
    
            return true;
        }
        int32_t GetConversion() noexcept
        {
            if (m_conversions.Empty()) return 0;

            int32_t output = average_buffer();
            output >>= EXTRA_BITS;
            output -= BIN_OFFSET;
            
            return output;
        }
        float ToVoltage(int32_t conversion) const noexcept
        {
            if (!conversion) return 0.0f;

            float volts;
            volts = (float)conversion;
            volts /= (float)(MAX_CODE + 1u);
            volts *= V_REF;

            return volts;
        }

    private:
        constexpr static float V_REF = 4.092f;
        constexpr static uint32_t MAX_CODE = 0xFFFFFF;
        constexpr static uint32_t BIN_OFFSET = 0x800000;
        constexpr static uint8_t EXTRA_BITS = 5u;
        constexpr static uint8_t DMA_DATA_SIZE = 2u;

    private:
        buffer_t m_conversions;
        OverSampling m_OSR;
        uint16_t m_dmaTxBuffer[DMA_DATA_SIZE];
        uint16_t m_dmaRxBuffer[DMA_DATA_SIZE];

    private:
        void spi_read_blocking() noexcept
        {
            LL_GPIO_ResetOutputPin(CS_Port, CS_Pin);
            LL_mDelay( 1U );
            for (int8_t i = 1; i >= 0; --i)
            {
                LL_SPI_TransmitData16(SPI, m_dmaTxBuffer[i]);
                while(!LL_SPI_IsActiveFlag_TXE(SPI));
        
                while (!LL_SPI_IsActiveFlag_RXNE(SPI));
                m_dmaRxBuffer[i] = LL_SPI_ReceiveData16(SPI);
            }
            LL_GPIO_SetOutputPin(CS_Port, CS_Pin);

            m_conversions.Push(build_conversion());
        }
        void dma_start_read() noexcept
        {
            LL_DMA_SetDataLength(DMA, RxChannel, DMA_DATA_SIZE);
            LL_DMA_SetDataLength(DMA, TxChannel, DMA_DATA_SIZE);

            LL_DMA_EnableChannel(DMA, RxChannel);
            LL_DMA_EnableChannel(DMA, TxChannel);

            LL_GPIO_ResetOutputPin(CS_Port, CS_Pin);
        }
        void dma_reset() noexcept
        {
            LL_GPIO_SetOutputPin(CS_Port, CS_Pin);
            
            if (LL_DMA_IsActiveFlag_TC4(DMA))
            {
                LL_DMA_ClearFlag_TC4(DMA);
                m_conversions.Push(build_conversion());
            }
            else if (LL_DMA_IsActiveFlag_TE4(DMA))
            {
                LL_DMA_ClearFlag_TE4(DMA);
            }

            LL_DMA_DisableChannel(DMA, RxChannel);
            LL_DMA_DisableChannel(DMA, TxChannel);
        }
        uint32_t average_buffer() noexcept
        {
            uint32_t output = 0;
            size_t cnt = 0;

            while (!m_conversions.Empty())
            {
                output += m_conversions.Pop();
                ++cnt;
            }

            output = (cnt == 4) ? (output >> 2) : (output / cnt);

            return output;
        }
        [[nodiscard]]uint32_t build_conversion() const
        {
            uint32_t output = ((m_dmaRxBuffer[0] << 16u) | m_dmaRxBuffer[1]);
            return output;
        }
    };
}
