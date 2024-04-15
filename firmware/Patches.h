#pragma once

#include "Patches2.h"

namespace Dexy {

/// @brief Patch definition and storage
namespace Patches {

// Use latest version of Patch struct definitions

/// @brief Envelope level scaling curve type
using ScalingCurve = V2::ScalingCurve;

/// @brief Envelope level scaling parameters
using LevelScalingParams = V2::LevelScalingParams;

/// @brief Envelope parameters
using EnvParams = V2::EnvParams;

/// @brief Operator parameters
using OpParams = V2::OpParams;

/// @brief Length of a patch's name
static constexpr unsigned patchNameLen = V2::patchNameLen;

/// @brief Patch name is a fixed-size array of characters (not 0-terminated)
using patchName_t = V2::patchName_t;

/// @brief Parameters of a single patch
using Patch = V2::Patch;

/// @brief Number of patches in a Dexy::Patches::PatchBank
constexpr unsigned numPatches = V2::numPatches;

/// @brief The set of patches that are stored in the module
/// @details There is one instance of PatchBank containing the current set of patches.
using PatchBank = V2::PatchBank;

// Sizes of serialized data objects
// These must be defined explicitly but the sizes are verified at compile time just in case.
// There are no variable-size data members in our serializable types.
// Note that these sizes are different from sizeof(x) because serialization packs differently.
constexpr size_t patchSize = V2::patchSize;
constexpr size_t patchBankDataSize = Serialize::serializeHdrSize + V2::patchBankSize;

constexpr bool isValid(const LevelScalingParams& obj);   ///< Does the object contain valid data?

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
using SerializedPatchBank = std::array<char, patchBankDataSize>;

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
