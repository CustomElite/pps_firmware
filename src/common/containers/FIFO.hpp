#pragma once

#include <cstddef>

namespace Containers
{
    template < typename T, size_t Length, bool Overwrite = false >
    class FIFO
    {
    public:
        using value_t = T;

        constexpr FIFO() = default;

        constexpr size_t Push(const T& input) noexcept
        {
            if (Full())
            {
                if (!OverwriteEnabled()) return 0;

                IncrementTail();
            }

            m_Buffer[Index(m_Head)] = input;
            IncrementHead();
            return 1u;
        }
        constexpr T Pop() noexcept
        {
            if (Empty()) return {};

            T output = m_Buffer[Index(m_Tail)];
            IncrementTail();
            return output;
        }
        constexpr void Clear() noexcept
        {
            m_Head = m_Tail = 0;
        }
        constexpr size_t Size() const noexcept { return (m_Head - m_Tail); }
        constexpr size_t Capacity() const noexcept { return Length; }
        constexpr size_t Space() const noexcept { return (Length - Size()); }
        constexpr bool Empty() const noexcept { return (Size() == 0); }
        constexpr bool Full() const noexcept { return (Size() == Length); }
        constexpr bool OverwriteEnabled() const noexcept { return Overwrite; }

    protected:
        T m_Buffer[Length] = {};
        size_t m_Head = 0;
        size_t m_Tail = 0;

        constexpr void Wrap() noexcept
        {
            while ((m_Head >= Length) && (m_Tail >= Length))
            {
                m_Head -= Length;
                m_Tail -= Length;
            }
        }
        constexpr size_t Index(size_t input) const noexcept
        {
            return (input % Length);
        }
        constexpr void IncrementHead() noexcept
        {
            ++m_Head;
            Wrap();
        }
        constexpr void IncrementTail() noexcept
        {
            ++m_Tail;
            Wrap();
        }
    };
}
