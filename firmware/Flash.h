#pragma once

namespace Dexy {

/// @brief Write persistent data to flash memory
namespace Flash {

// Definitions for persistent data stored in flash memory

/// @brief Round up a size_t to a multiple of a given chunk size
/// @param size The size of a thing in memory
/// @param chunkSize The size of a memory chunk
/// @return A size_t which is >= size and a multiple of chunkSize
consteval size_t FLASH_SIZE_ADJUST(size_t size, size_t chunkSize)
    { return ((size + chunkSize - 1) / chunkSize) * chunkSize; }

/// @brief Round up a size_t to a multiple of the flash memory page size
/// @param size The size of a thing in memory
/// @return A size_t which is >= size and a multiple of FLASH_PAGE_SIZE
consteval size_t FLASH_PAGE_ADJUST(size_t size)
    { return FLASH_SIZE_ADJUST(size, FLASH_PAGE_SIZE); }

/// @brief Round up a size_t to a multiple of the flash memory sector size
/// @param size The size of a thing in memory
/// @return A size_t which is >= size and a multiple of FLASH_SECTOR_SIZE
consteval size_t FLASH_SECTOR_ADJUST(size_t size)
    { return FLASH_SIZE_ADJUST(size, FLASH_SECTOR_SIZE); }

/// @brief A wrapper class that holds an object, is aligned with a flash
/// memory sector, and fully occupies one or more sectors.
/// @tparam T The type that is wrapped
template<typename T>
union alignas(FLASH_SECTOR_SIZE) Wrapper {
    T obj; ///< The object that is wrapped
    char placeholder[FLASH_SECTOR_ADJUST(sizeof(T))];
};

/// @brief Copy an object to flash memory
/// @tparam T The type of object being copied
/// @param objFrom The object being copied
/// @param[out] pobjTo The destination to copy to, which is a Dexy::Flash::Wrapper that is
/// located in flash memory
/// @details This function is loaded into RAM (by project compile settings)
/// and declared noinline so it will not be executed from flash.
template<typename T>
__attribute__((noinline))
void copyToFlash(const T& objFrom, Wrapper<T>* pobjTo);

} } // namespace Flash
