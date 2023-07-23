#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#pragma GCC diagnostic ignored "-Wctad-maybe-unsupported"
#endif
#include "zpp_bits.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace Dexy {

/// @brief Data serialization and deserialization using the zpp::bits library
namespace Serialize {

// Sizes of serialized data objects
// These must be defined explicitly but the sizes are verified at compile time just in case.
// There are no variable-size data members in our serializable types.
// Note that these sizes are different from sizeof(x) because serialization packs differently.
constexpr size_t serializeHdrSize = 6;
constexpr size_t envParamsSize = 11;
constexpr size_t opParamsSize = 8 + envParamsSize;
constexpr size_t patchSize = 16 + 3 + 6 * opParamsSize;
constexpr size_t patchBankSize = 32 * patchSize;
constexpr size_t opParamsChangeDataSize = serializeHdrSize + 5;
constexpr size_t patchSettingChangeDataSize = serializeHdrSize + 4;
constexpr size_t patchNameChangeDataSize = serializeHdrSize + 1 + 16;
constexpr size_t patchChangeDataSize = serializeHdrSize + 1 + patchSize;
constexpr size_t patchBankDataSize = serializeHdrSize + patchBankSize;

/// @brief Type of serializeCookie
using cookie_t = uint32_t;

/// @brief Magic cookie that identifies serialized data packets
constexpr cookie_t serializeCookie = 'D'|('e'|('x'|('y'<<8))<<8)<<8; // little-endian

/// @brief Type of serializeVersion
using version_t = uint16_t;

/// @brief Version number for the serialized data format
constexpr version_t serializeVersion = 1;

/// @brief Read an object from a serialized data blob
/// @param storage 
/// @param[out] pobj 
/// @return Success
IN_FLASH("Serialize")
bool readObject(const auto& storage, auto* pobj)
{
    //dassert(pobj != nullptr, BadArgument);
    cookie_t cookie;
    version_t version;
    auto in = zpp::bits::in(storage);
    if (success(in(cookie))
        && cookie == serializeCookie
        && success(in(version))
        && version == serializeVersion
        && success(in(*pobj)))
    {
        return true;
    } else {
        dputs("readObject: ERROR: Bad patch data");
        Error::set<Error::Err::BadPatchData>();
        return false;
    }
}

/// @brief Write an object to a serialized data blob
/// @param storage 
/// @param obj 
/// @return Number of bytes written
IN_FLASH("Serialize")
size_t writeObject(auto& storage, const auto& obj)
{
    auto out = zpp::bits::out(storage);
    if (success(out(serializeCookie))
        && success(out(serializeVersion))
        && success(out(obj)))
    {
        return out.position();
    } else {
        return 0;
    }
}

// #kludge
/// @brief Customized versions of some zpp::bits functions with more parameters
namespace my_bits
{
    template <auto Object, typename Type, std::size_t MaxSize = 0x1000>
    static constexpr auto to_bytes_one()
    {
        using namespace zpp::bits;
        constexpr auto size = [] {
            std::array<Type, MaxSize> data;
            out out{data};
            out(Object).or_throw();
            return out.position();
        }();

        if constexpr (!size) {
            return string_literal<Type, 0>{};
        } else {
            std::array<Type, size> data;
            out{data}(Object).or_throw();
            return data;
        }
    }

    template <typename Type, auto... Data>
    static constexpr auto join()
    {
        using namespace zpp::bits;
        constexpr auto size = (0 + ... + Data.size());
        if constexpr (!size) {
            return string_literal<Type, 0>{};
        } else {
            std::array<Type, size> data;
            out{data}(Data...).or_throw();
            return data;
        }
    }

    template <typename Type, std::size_t MaxSize, auto... Object>
    static constexpr auto to_bytes()
    {
        return join<Type, to_bytes_one<Object, Type, MaxSize>()...>();
    }
} // namespace my_bits

/// @brief Consteval serialization for compile-time object initialization
/// @tparam T Type of object to serialize
/// @tparam obj Object to serialize
/// @tparam SIZE Size of serialized data
/// @return Serialized data representing obj
template<typename T, T obj, size_t SIZE>
consteval std::array<char, SIZE> objToBytes()
{
    return my_bits::to_bytes<char, 8000, serializeCookie, serializeVersion, obj>();
}

} } // namespace Serialize
