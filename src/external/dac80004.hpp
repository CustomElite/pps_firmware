#pragma once

#include "stm32f1xx_ll_spi.h"

#include "peripherals/gpio.hpp"

#include <utility>
#include <cstdint>

namespace External::DAC80004
{
    enum class Channel : uint8_t
    {
        A           = 0,
        B,
        C,
        D,
        ALL         = 0b1111
    };

    enum class PowerState : uint8_t
    {
        On          = 0,
        Off_1k,
        Off_100K,
        Off_HiZ
    };

    enum class ClearMode : uint8_t
    {
        ZeroScale   = 0,
        MidScale,
        FullScale
    };

    enum class LoadMode : uint8_t
    {
        Pin         = 0,
        Auto
    };

    enum class CurrentLimit : uint8_t
    {
        IMax_40ma    = 0,
        IMax_30ma
    };

    enum class SDO_State : uint8_t
    {
        Off          = 0,
        On           = 0b10
    };

    enum class Command : uint8_t
    {
      Set                 = 0,
      Update              = 0x01,
      SetAndLoad          = 0x02,
      SetAndUpdate        = 0x03,
      WritePowerState     = 0x04,
      WriteClearMode      = 0x05,
      WriteLDAC           = 0x06,
      ResetRegisters      = 0x07,
      WriteSDO            = 0x08,
      WriteCurrentMode    = 0x0A,
      Clear               = 0x0B,
      NOP                 = 0x0E,
      ReadStatus          = 0x1D
    };

    struct Properties
    {
        SPI_TypeDef* SPIx;
        GPIO_TypeDef* SyncPort; uint16_t SyncPin;

        Properties(SPI_TypeDef* spi, GPIO_TypeDef* sync_port, uint32_t sync_pin)
            : SPIx(spi), SyncPort(sync_port), SyncPin(sync_pin)
        {
            
        }
    };

    struct CommandBuilder
    {
        constexpr static uint8_t const CommandPos = 24u;
        constexpr static uint8_t const ChannelPos = 20u;
        constexpr static uint8_t const ValuePos = 4u;

        template <Channel CH>
        inline static uint16_t* Set(uint16_t value) noexcept
        {
            uint32_t output = ((Command::Set << CommandPos) | (CH << ChannelPos) | (value << ValuePos))
        } 
    };

    class DAC80004 : Properties
    {
    public:
        template <typename... Args>
        DAC80004(Args&&... args) : Properties{ std::forward<Args>(args)... }
        {

        }

    private:
        constexpr static uint16_t const MAX_CODE    = 0xFFFF;
        constexpr static float const VREF           = 4.092f;
        constexpr static float const STEP_SIZE      = (VREF / (MAX_CODE + 1u));

        

    private:

    };
}
