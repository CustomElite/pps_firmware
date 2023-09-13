#pragma once

#include <cstdint>

#include "stm32f1xx_ll_spi.h"

#include "peripherals/pin.hpp"

namespace Peripheral::SPI
{
    enum class Module : uint32_t
    {
        SPI_1 = SPI1_BASE,
        SPI_2 = SPI2_BASE
    };

    template <typename CS_Pin, typename MOSI_Pin, typename MISO_Pin, typename SCLK_Pin>
    struct Pins
    {
        CS_Pin CS;
        MOSI_Pin MOSI;
        MISO_Pin MISO;
        SCLK_Pin SCLK;

        Pins() noexcept 
          : CS{IO::Mode::Output, IO::Type::PushPull, IO::Speed::High}, 
          MOSI{IO::Mode::Alternate, IO::Type::PushPull, IO::Speed::High}, 
          MISO{IO::Mode::Floating}, 
          SCLK{IO::Mode::Alternate, IO::Type::PushPull, IO::Speed::High}
        {
        }
    };



    template <Module base, typename... pins>
    class SPI_Kernal : public Pins<pins...>
    {
    private:
        static constexpr SPI_TypeDef* SPIx = (SPI_TypeDef*)((uint32_t)base);

    public:
        SPI_Kernal(const LL_SPI_InitTypeDef& config) noexcept
        {
            switch (base)
            {
            case Module::SPI_1:
                LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
                break;
            case Module::SPI_2:
                LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
                break;
            }

            LL_SPI_Init(SPIx, &config);
            
        }
        ~SPI_Kernal() noexcept
        {
            LL_SPI_DeInit(SPIx);

            switch (base)
            {
            case Module::SPI_1:
                LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_SPI1);
                break;
            case Module::SPI_2:
                LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_SPI2);
                break;
            }
        }
        inline static void Enable() noexcept
        {
            LL_SPI_Enable(SPIx);
        }
        inline static void Disable() noexcept
        {
            LL_SPI_Disable(SPIx);
        }
        inline static void EnableDMA_RX() noexcept
        {
            LL_SPI_EnableDMAReq_RX(SPIx);
        }
        inline static void EnableDMA_TX() noexcept
        {
            LL_SPI_EnableDMAReq_TX(SPIx);
        }
        inline static size_t TransmitBlock(uint8_t* rx_buffer, uint8_t* tx_buffer, size_t block_size) noexcept
        {
            size_t count = 0;

            CS = IO::State::Low;
            for (; count < block_size; ++count)
            {
                LL_SPI_TransmitData8(SPIx, *(tx_buffer + count));
                while (!LL_SPI_IsActiveFlag_TXE(SPIx));

                while (!LL_SPI_IsActiveFlag_RXNE(SPIx));
                *(rx_buffer + count) = LL_SPI_ReceiveData8(SPIx);
            }
            CS = IO::State::High;

            return count;
        }
    };
    
}
