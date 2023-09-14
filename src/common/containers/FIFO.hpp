#pragma once

#include <cstddef>

namespace Containers
{
    template < typename T, size_t Length, bool Overwrite = false >
    class FIFO
    {
    public:
        using value_t = T;
        using iterator = value_t*;

        constexpr FIFO() = default;

        constexpr size_t Push(const T& input) noexcept
        {
            if (Full())
            {
                if (Overwrite == false) return 0;

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
        iterator begin() noexcept { return (m_Buffer + Index(m_Tail)); }
        iterator end() noexcept { return (m_Buffer + Index(m_Head)); }
        [[nodiscard]] constexpr size_t Size() const noexcept { return (m_Head - m_Tail); }
        [[nodiscard]] constexpr size_t Capacity() const noexcept { return Length; }
        [[nodiscard]] constexpr size_t Space() const noexcept { return (Length - Size()); }
        [[nodiscard]] constexpr bool Empty() const noexcept { return (Size() == 0); }
        [[nodiscard]] constexpr bool Full() const noexcept { return (Size() == Length); }

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
        [[nodiscard]] constexpr size_t Index(size_t input) noexcept
        {
            return (input % Length);
        }
    };
}
