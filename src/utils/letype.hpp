/*
 * This file was taken from Cemu emulator's betype and modified a little
 * */

#pragma once
#include <wut_types.h>
#include <type_traits>
#include <variant>
#include <climits>

namespace Latte
{
    class LATTEREG;
};

template<class T, std::size_t... N>
constexpr T bswap_impl(T i, std::index_sequence<N...>)
{
    return ((((i >> (N * CHAR_BIT)) & (T)(uint8_t)(-1)) << ((sizeof(T) - 1 - N) * CHAR_BIT)) | ...);
}

template<class T, class U = std::make_unsigned_t<T>>
constexpr T bswap(T i)
{
    return (T)bswap_impl<U>((U)i, std::make_index_sequence<sizeof(T)>{});
}

template <typename T>
constexpr T SwapEndian(T value)
{
    if constexpr (std::is_integral<T>::value)
    {
        return (T)bswap((std::make_unsigned_t<T>)value);
    }
    else if constexpr (std::is_floating_point<T>::value)
    {
        if constexpr (sizeof(T) == sizeof(uint32_t))
        {
            const auto tmp = bswap<uint32_t>(*(uint32_t*)&value);
            return *(T*)&tmp;
        }
        if constexpr (sizeof(T) == sizeof(uint64_t))
        {
            const auto tmp = bswap<uint64_t>(*(uint64_t*)&value);
            return *(T*)&tmp;
        }
    }
    else if constexpr (std::is_enum<T>::value)
    {
        return (T)SwapEndian((std::underlying_type_t<T>)value);
    }
    else if constexpr (std::is_base_of<Latte::LATTEREG, T>::value)
    {
        const auto tmp = bswap<uint32_t>(*(uint32_t*)&value);
        return *(T*)&tmp;
    }
    else
    {
        static_assert(std::is_integral<T>::value, "unsupported letype specialization!");
    }

    return value;
}


// swap if native isn't big endian
template <typename T>
constexpr T _BE(T value)
{
    return value;
}

// swap if native isn't little endian
template <typename T>
constexpr T _LE(T value)
{
    return SwapEndian(value);
}

template <typename T>
class letype
{
public:
    constexpr letype() = default;

    // copy
    constexpr letype(T value)
            : m_value(SwapEndian(value)) {}

    constexpr letype(const letype& value) = default; // required for trivially_copyable
    //constexpr letype(const letype& comparison_value)
    //	: m_value(comparison_value.m_value) {}

    template <typename U>
    constexpr letype(const letype<U>& value)
            : letype((T)value.value()) {}

    // assigns
    static letype from_bevalue(T value)
    {
        letype result;
        result.m_value = value;
        return result;
    }

    // returns LE comparison_value
    constexpr T value() const { return SwapEndian<T>(m_value); }

    // returns BE comparison_value
    constexpr T bevalue() const { return m_value; }

    constexpr operator T() const { return value(); }

    letype<T>& operator+=(const letype<T>& v)
    {
        m_value = SwapEndian(T(value() + v.value()));
        return *this;
    }

    letype<T>& operator-=(const letype<T>& v)
    {
        m_value = SwapEndian(T(value() - v.value()));
        return *this;
    }

    letype<T>& operator*=(const letype<T>& v)
    {
        m_value = SwapEndian(T(value() * v.value()));
        return *this;
    }

    letype<T>& operator/=(const letype<T>& v)
    {
        m_value = SwapEndian(T(value() / v.value()));
        return *this;
    }

    letype<T>& operator&=(const letype<T>& v) requires (requires (T& x, const T& y) { x &= y; })
    {
        m_value &= v.m_value;
        return *this;
    }

    letype<T>& operator|=(const letype<T>& v) requires (requires (T& x, const T& y) { x |= y; })
    {
        m_value |= v.m_value;
        return *this;
    }

    letype<T>& operator|(const letype<T>& v) const requires (requires (T& x, const T& y) { x | y; })
    {
        letype<T> tmp(*this);
        tmp.m_value = tmp.m_value | v.m_value;
        return tmp;
    }

    letype<T> operator|(const T& v) const requires (requires (T& x, const T& y) { x | y; })
    {
        letype<T> tmp(*this);
        tmp.m_value = tmp.m_value | SwapEndian(v);
        return tmp;
    }

    letype<T>& operator^=(const letype<T>& v) requires std::integral<T>
    {
        m_value ^= v.m_value;
        return *this;
    }

    letype<T>& operator>>=(std::size_t idx) requires std::integral<T>
    {
        m_value = SwapEndian(T(value() >> idx));
        return *this;
    }

    letype<T>& operator<<=(std::size_t idx) requires std::integral<T>
    {
        m_value = SwapEndian(T(value() << idx));
        return *this;
    }

    letype<T> operator~() const requires std::integral<T>
    {
        return from_bevalue(T(~m_value));
    }

    letype<T>& operator++() requires std::integral<T>
    {
        m_value = SwapEndian(T(value() + 1));
        return *this;
    }

    letype<T>& operator--() requires std::integral<T>
    {
        m_value = SwapEndian(T(value() - 1));
        return *this;
    }
private:
    //ContiguousContainer m_value{}; // before 1.26.2
    T m_value;
};

using uint64le = letype<uint64_t>;
using uint32le = letype<uint32_t>;
using uint16le = letype<uint16_t>;
using uint8le = letype<uint8_t>;

using int64le = letype<int64_t>;
using int32le = letype<int32_t>;
using int16le = letype<int16_t>;
using int8le = letype<int8_t>;

using float32le = letype<float>;
using float64le = letype<double>;

static_assert(sizeof(letype<uint64_t>) == sizeof(uint64_t));
static_assert(sizeof(letype<uint32_t>) == sizeof(uint32_t));
static_assert(sizeof(letype<uint16_t>) == sizeof(uint16_t));
static_assert(sizeof(letype<uint8_t>) == sizeof(uint8_t));
static_assert(sizeof(letype<float>) == sizeof(float));
static_assert(sizeof(letype<double>) == sizeof(double));

static_assert(std::is_trivially_copyable_v<uint32le>);
static_assert(std::is_trivially_constructible_v<uint32le>);
static_assert(std::is_copy_constructible_v<uint32le>);
static_assert(std::is_move_constructible_v<uint32le>);
static_assert(std::is_copy_assignable_v<uint32le>);
static_assert(std::is_move_assignable_v<uint32le>);