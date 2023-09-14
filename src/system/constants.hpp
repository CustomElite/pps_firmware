#include <cstdint>

namespace System
{
    /*
     * Clock Frequency */
    constexpr static uint32_t const LSE_Clock           = 32'768u;
    constexpr static uint32_t const LSI_Clock           = 40'000u;
    constexpr static uint32_t const HSEClock            = 8'000'000u;
    constexpr static uint32_t const HSI_Clock           = 8'000'000u;



    constexpr static uint32_t const VREF                = 4.092f;
    constexpr static uint32_t const VOUT_Tolerance      = 1.0f;
    constexpr static uint32_t const IOUT_Tolerance      = 1.0f;
}
