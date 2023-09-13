#pragma once

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_spi.h"

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
        IMax_40ma   = 0,
        IMax_30ma
    };

    enum class SDO_State : uint8_t
    {
        Off         = 0,
        On          = 0b10
    };

    struct Properties
    {
        SPI_TypeDef* SPIx;
        GPIO_TypeDef* SyncPort; uint32_t SyncPin;

        Properties(SPI_TypeDef* spi, GPIO_TypeDef* sync_port, uint32_t sync_pin)
            : SPIx(spi), SyncPort(sync_port), SyncPin(sync_pin)
        {
            
        }
    };
#if 0
    class Command
    {
    protected:
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t Set() noexcept
        {
            return ((eCMD::Set << CMD_Pos) | ((uint8_t)CH << CH_Pos) | (VAL << VAL_Pos));
        }
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t Update() noexcept
        {
            return ((eCMD::Update << CMD_Pos) | ((uint8_t)CH << CH_Pos) | (VAL << VAL_Pos));
        }
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t SetAndLoad() noexcept
        {
            return ((eCMD::SetAndLoad << CMD_Pos) | ((uint8_t)CH << CH_Pos) | (VAL << VAL_Pos));
        }
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t SetAndUpdate() noexcept
        {
            return ((eCMD::SetAndUpdate << CMD_Pos) | ((uint8_t)CH << CH_Pos) | (VAL << VAL_Pos));
        }
        template <PowerState Power, bool CH_A, bool CH_B, bool CH_C, bool CH_D>
        inline static constexpr uint32_t PowerState() noexcept
        {
            return ((eCMD::WritePowerState << CMD_Pos) | ((uint8_t)Power << PWR_Pos) | (((uint8_t)CH_D << 3u) | ((uint8_t)CH_C << 2u) | ((uint8_t)CH_B << 1u) | ((uint8_t)CH_A)));
        }
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t Reset() noexcept
        {
            return (eCMD::ResetRegisters << CMD_Pos);
        }
        template <Channel CH, uint16_t VAL>
        inline static constexpr uint32_t Clear() noexcept
        {
            return (eCMD::Clear << CMD_Pos);
        }

    private:
        enum eCMD : uint8_t
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

        constexpr static uint8_t const CMD_Pos = 24u;
        constexpr static uint8_t const CH_Pos = 20u;
        constexpr static uint8_t const PWR_Pos = 8u;
        constexpr static uint8_t const VAL_Pos = 4u;
    };
#endif

    class DAC80004 : public Properties
    {
    public:
        template <typename... Args>
        DAC80004(Args&&... args) : Properties(args...)
        {

        }
        void Enable() noexcept
        {
            LL_SPI_Enable(SPIx);
        }
        void Disable() noexcept
        {
            LL_SPI_Disable(SPIx);
        }
        template <typename... Args>
        void Set(Channel channel, uint16_t value) noexcept
        {
            spi_tx_command(CommandBuilder(Commands::Set, channel, value));
        }
        void Update(Channel channel) noexcept
        {
            spi_tx_command(CommandBuilder(Commands::Update, channel));
        }
        void SetAndUpdate(Channel channel, uint16_t value) noexcept
        {
            spi_tx_command(CommandBuilder(Commands::SetAndUpdate, channel, value));
        }
        void SetAndLoadAll(Channel channel, uint16_t value) noexcept
        {
            spi_tx_command(CommandBuilder(Commands::SetAndLoad, channel, value));
        }
        void SetPowerState(PowerState state, bool ch_a, bool ch_b, bool ch_c, bool ch_d) noexcept
        {
            spi_tx_command(CommandBuilder(state, ch_a, ch_b, ch_c, ch_d));
        }

    private:
        constexpr static uint16_t const MAX_CODE    = 0xFFFF;
        constexpr static float const VREF           = 4.092f;
        constexpr static float const STEP_SIZE      = (VREF / (MAX_CODE + 1u));

        enum class Commands : uint8_t
        {
            Set                 = 0,
            Update              = 0x01,
            SetAndLoad          = 0x02,
            SetAndUpdate        = 0x03,
            PowerState          = 0x04,
            ClearMode           = 0x05,
            LDAC                = 0x06,
            ResetRegisters      = 0x07,
            SDO                 = 0x08,
            CurrentMode         = 0x0A,
            Clear               = 0x0B,
            NOP                 = 0x0E,
            GetStatus           = 0x1D
        };

        struct CommandBuilder
        {
            union 
            {
                uint32_t U32 = 0;
                struct {
                    uint16_t LSB;
                    uint16_t MSB;
                } U16;
                struct {
                    uint32_t      : 4;
                    uint32_t VAL  : 16;
                    uint32_t CH   : 4;
                    uint32_t CMD  : 4;
                    uint32_t RW   : 4;
                } OUT;
                struct {
                    uint32_t MODE : 4;
                    uint32_t RDY  : 1;
                    uint32_t      : 3;
                    uint32_t PD   : 2;
                    uint32_t      : 14;
                    uint32_t CMD  : 4;
                    uint32_t RW   : 4;
                } CMD;
            } Command;

            CommandBuilder(Commands cmd) noexcept
            {
                Command.CMD.CMD = (uint8_t)cmd;
            }
            CommandBuilder(ClearMode clear_mode) noexcept
            {
                Command.CMD.CMD = (uint8_t)Commands::ClearMode;
                Command.CMD.MODE = (uint8_t)clear_mode;
            }
            CommandBuilder(SDO_State sdo_state) noexcept
            {
                Command.CMD.CMD = (uint8_t)Commands::SDO;
                Command.CMD.MODE = (uint8_t)sdo_state;
            }
            CommandBuilder(Commands cmd, Channel channel, uint16_t value = 0) noexcept
            {
                Command.OUT.CMD = (uint8_t)cmd;
                Command.OUT.CH = (uint8_t)channel;
                Command.OUT.VAL = value;
            }
            CommandBuilder(PowerState state, bool ch_a, bool ch_b, bool ch_c, bool ch_d) noexcept
            {
                Command.CMD.CMD = (uint8_t)Commands::PowerState;
                Command.CMD.PD = (uint8_t)state;
                Command.CMD.MODE = ((ch_d << 3u) | (ch_c << 2u) | (ch_b << 1u) | ch_a);
            }
            CommandBuilder(LoadMode ch_a, LoadMode ch_b, LoadMode ch_c, LoadMode ch_d) noexcept
            {
                Command.CMD.CMD = (uint8_t)Commands::LDAC;
                Command.CMD.MODE = (((uint8_t)ch_d << 3u) | ((uint8_t)ch_c << 2u) | ((uint8_t)ch_b << 1u) | (uint8_t)ch_a);
            }
            CommandBuilder(CurrentLimit ch_a, CurrentLimit ch_b, CurrentLimit ch_c, CurrentLimit ch_d) noexcept
            {
                Command.CMD.CMD = (uint8_t)Commands::CurrentMode;
                Command.CMD.MODE = (((uint8_t)ch_d << 3u) | ((uint8_t)ch_c << 2u) | ((uint8_t)ch_b << 1u) | (uint8_t)ch_a);
            }
        };

    private:
        void spi_tx_command(CommandBuilder&& input) noexcept
        {
            LL_GPIO_ResetOutputPin(SyncPort, SyncPin);

            LL_SPI_TransmitData16(SPIx, (uint16_t)input.Command.U16.MSB);
            while (!LL_SPI_IsActiveFlag_TXE(SPIx));
            
            while (!LL_SPI_IsActiveFlag_RXNE(SPIx));
            LL_SPI_ReceiveData16(SPIx);

            LL_SPI_TransmitData16(SPIx, (uint16_t)input.Command.U16.LSB);
            while (!LL_SPI_IsActiveFlag_TXE(SPIx));

            while (!LL_SPI_IsActiveFlag_RXNE(SPIx));
            LL_SPI_ReceiveData16(SPIx);

            LL_GPIO_SetOutputPin(SyncPort, SyncPin);
        }
    };
}
