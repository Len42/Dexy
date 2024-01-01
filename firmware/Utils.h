#pragma once

namespace Dexy {

// Utility Functions

// Pico SDK utilities

/// @brief Use this when assigning a value to a hardware register
/// (io_rw_32, io_wo_32)
/// @param val 
/// @return val cast to uint32_t
constexpr uint32_t as_reg32(auto val)
{
    return reinterpret_cast<uint32_t>(val);
}

/// @brief constexpr version of pwm_gpio_to_slice_num()
/// @param gpio GPIO pin number
/// @return PWM slice number
constexpr uint pwm_gpio_to_slice_num_constexpr(uint gpio)
{
    //assert(gpio < NUM_BANK0_GPIOS);
    return (gpio >> 1u) & 7u;
}

/// @brief constexpr version of to_us_since_boot()
/// @param t Time as absolute_time_t
/// @return Time as a uint64_t
constexpr uint64_t to_us_since_boot_constexpr(absolute_time_t t)
{
#ifdef NDEBUG
    return t;
#else
    return t._private_us_since_boot;
#endif
}

/// @brief constexpr version of from_us_since_boot()
/// @param t Time as uint64_t
/// @return Time as a absolute_time_t
constexpr absolute_time_t from_us_since_boot_constexpr(uint64_t t)
{
    absolute_time_t tRet;
#ifdef NDEBUG
    tRet = t;
#else
    //assert(us_since_boot <= INT64_MAX);
    tRet._private_us_since_boot = t;
#endif
    return tRet;
}

/// @brief Compare two times
/// @details Just a simple 64-bit comparison, but works in Debug build where
/// absolute_time_t is defined differently.
/// @param t Time to compare, often from get_absolute_time()
/// @param tTarget Deadline or timeout to compare t to
/// @return true if t is on or after tTarget, false otherwise
constexpr bool timeIsReached(absolute_time_t t, absolute_time_t tTarget)
{
    return to_us_since_boot_constexpr(t) >= to_us_since_boot_constexpr(tTarget);
}

// Bit-twiddling utilities

/// @brief Compile-time function to calculate the number of bits required to
/// represent the given unsigned value
/// @note This relies on std::log2() being declared constexpr,
/// which it is in gcc but not in other compilers or the C++20 standard. :(
/// @param val 
/// @return The number of bits required to represent val
consteval unsigned bits_in_num(unsigned val)
{
    return static_cast<unsigned>(std::log2(val) + 1);
}

/// @brief Compile-time function to make a bit-mask word with the lower bits = 1
/// and the upper bits = 0
/// @param numBits The number of low-order bits to set to 1
/// @return Bit mask
consteval unsigned mask_low_bits(int numBits)
{
    unsigned val = 0;
    for (int i = 0; i < numBits; ++i) {
        val |= (1 << i);
    }
    return val;
}

/// @brief Helper for bitmask()
constexpr unsigned bitmaskHelper(unsigned mask) { return mask; }

/// @brief Helper for bitmask()
template<typename... Args>
constexpr unsigned bitmaskHelper(unsigned mask, unsigned bit, Args... bits)
    { return bitmaskHelper(mask | (1 << bit), bits...); }

/// @brief Return a bit-mask word with the given bits set
/// @details Bits are specified by index, e.g. bitmask(0, 2, 7) == 0b10000101.
/// @tparam ...Args 
/// @param ...bits Bit indices
/// @return Bit mask
template<typename... Args>
constexpr unsigned bitmask(Args... bits)
{
    return bitmaskHelper(0, bits...);
}

// C++ utilities

/// @brief Just an alias for std::exchange because no-one can remember what it
/// does from its name
#define getAndSet std::exchange

/// @brief For a given numeric type, return the value midway between the minimum
/// and maximum representable values.
/// @tparam T A numeric type
/// @return The middle value in the range of T's values
template<typename T>
consteval T mid_value()
{
    return static_cast<T>((std::numeric_limits<T>::min() + std::numeric_limits<T>::max()) / 2);
}

/// @brief Convert a std::array of chars to a std::string_view
/// @note There is a std::string_view constructor for this in C++ 23.
/// @tparam SIZE 
/// @param str 
/// @return 
template<size_t SIZE>
std::string_view toStringView(const std::array<char,SIZE>& str)
{
    return std::string_view(str.data(), SIZE);
}

/// @brief Compile-time conversion of a 0-terminated string constant to a
/// std::array<char>
/// @tparam SIZE String length deduced from the str argument
/// @param str 0-terminated string constant
/// @return The string as an array of char that is not 0-terminated
template<size_t SIZE>
consteval std::array<char, SIZE-1>
strToArray(const char(&str)[SIZE])
{
    std::array<char, SIZE-1> arr;
    std::copy(std::begin(str), std::prev(std::end(str)), std::begin(arr));
    return arr;
}

}
