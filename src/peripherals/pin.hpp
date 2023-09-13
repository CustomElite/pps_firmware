#pragma once

#include <cstdint>

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"

namespace IO
{
    enum class Port : uint32_t
    {
        A = 0,
        B,
        C,
        D,
        E
    };
    enum class State : bool
    {
        Low = false,
        High = true
    };
    enum class Mode : uint32_t
    {
        Analog = 0,
        Output = 0b0001,
        Floating = 0b0100,
        Input = 0b1000,
        Alternate = 0b1001
    };
    enum class Type : uint8_t
    {
        PushPull = 0,
        OpenDrain = 0b0100,
    };
    enum class Speed : uint32_t
    {
        Medium = 0b01,
        Low,
        High
    };
    enum class Resistors : uint32_t
    {
        PullDown = 0,
        PullUp
    };

    template <Port port, uint32_t pin>
    class Pin
    {
    public:
        static constexpr uint32_t PinIndex = pin;
        static constexpr uint32_t PortIndex = (uint32_t)port;

    protected:
        using port_t = GPIO_TypeDef*;
        static constexpr uint32_t rcc_mask = (1u << (PortIndex + 2u));
        static constexpr std::uint32_t pin_pos = (1u << pin);
        static constexpr uint32_t pin_mask = (pin < 8u) ? ((pin_pos << 8u) | pin_pos) : ((pin_pos << 8u) | (0x04000000 | (1u << (pin - 8u))));

        inline static constexpr auto GetPort() noexcept
        {
            return (port_t)( GPIOA_BASE + ( GPIOB_BASE - GPIOA_BASE ) * PortIndex);
        }
        inline static void EnablePort() noexcept
        {
            LL_APB2_GRP1_EnableClock(rcc_mask);
        }
        inline static void DisablePort() noexcept
        {
            LL_APB2_GRP1_DisableClock(rcc_mask);
        }
        inline static void Set(Mode mode) noexcept
        {
            LL_GPIO_SetPinMode(GetPort(), pin_mask, (uint32_t)mode);
        }
        inline static void Set(Type type) noexcept
        {
            LL_GPIO_SetPinOutputType(GetPort(), pin_mask, (uint32_t)type);
        }
        inline static void Set(Speed speed) noexcept
        {
            LL_GPIO_SetPinSpeed(GetPort(), pin_mask, (uint32_t)speed);
        }
        inline static void Set(Resistors resistors) noexcept
        {
            LL_GPIO_SetPinPull(GetPort(), pin_mask, (uint32_t)resistors);
        }
        inline static void Set(Mode mode, Resistors resistors) noexcept
        {
            Set(mode);
            Set(resistors);
        }
        inline static void Set(Mode mode, Type type, Speed speed) noexcept
        {
            Set(mode);
            Set(type);
            Set(speed);
        }
        inline static void Set(bool state) noexcept
        {
            if (state)
                LL_GPIO_SetOutputPin(GetPort(), pin_mask);
            else
                LL_GPIO_ResetOutputPin(GetPort(), pin_mask);
        }
        inline static void Set(State input) noexcept
        {
            Set(input == State::High);
        }

    public:
        template <typename... Args>
        inline Pin(Args... args) noexcept
        {
            Pin::EnablePort();
            Pin::Set(args...);
        }
        template <typename T>
        inline Pin& operator=(const T input) noexcept
        {
            Set(input);
            return *this;
        }
        inline static void Toggle() noexcept
        {
            LL_GPIO_TogglePin(GetPort(), pin_mask);
        }
    };
}
