#pragma once

#include "Patches1.h"

namespace Dexy {

/// @brief Patch definition and storage
namespace Patches {

// Use latest version of Patch struct definitions

/// @brief Envelope parameters
using EnvParams = V1::EnvParams;

/// @brief Operator parameters
using OpParams = V1::OpParams;

/// @brief Length of a patch's name
static constexpr unsigned patchNameLen = V1::patchNameLen;

/// @brief Patch name is a fixed-size array of characters (not 0-terminated)
using patchName_t = V1::patchName_t;

/// @brief Parameters of a single patch
using Patch = V1::Patch;

/// @brief Number of patches in a Dexy::Patches::PatchBank
constexpr unsigned numPatches = V1::numPatches;

/// @brief The set of patches that are stored in the module
/// @details There is one instance of PatchBank containing the current set of patches.
using PatchBank = V1::PatchBank;

constexpr bool isValid(const EnvParams& obj);   ///< Does the object contain valid data?

constexpr bool isValid(const OpParams& obj);    ///< Does the object contain valid data?

constexpr bool isValid(const Patch& obj);       ///< Does the object contain valid data?

constexpr bool isValid(const PatchBank& obj);   ///< Does the object contain valid data?

/// @brief Initialization - must be called at startup
void init();

/// @brief Get the specified patch
/// @param i Patch index
/// @return Patch
Patch& getPatch(unsigned i);

/// @brief Serialized data blob containing a PatchBank
using SerializedPatchBank = std::array<char, Serialize::patchBankDataSize>;

/// @brief Load the current PatchBank from a serialized data blob
/// @param storage 
/// @return Success
bool loadCurrentPatchBank(const auto& storage);

/// @brief Save the current PatchBank in a serialized data blob
/// @param[out] pstorage 
/// @return The number of bytes written
size_t saveCurrentPatchBank(auto* pstorage);

/// @brief Write a serialized PatchBank to persistent storage (flash memory)
/// @param patchData Serialized PatchBank data
void saveInitialPatchData(const SerializedPatchBank& patchData);

} } // namespace Patches
