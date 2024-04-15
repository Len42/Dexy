#pragma once

namespace Dexy { namespace Patches {

// Support for live updating of patches

/// @brief Patch update data - Replace an entire Patch in the current PatchBank
struct PatchChange {
    uint8_t iPatch;     ///< Patch number
    Patch patch;        ///< New Patch
};

/// @brief Patch update data - Patch name has changed
struct PatchNameChange {
    uint8_t iPatch;     ///< Patch number
    patchName_t name;   ///< New name
};

/// @brief Patch update data - A patch-wide setting has changed
struct PatchSettingChange {
    uint8_t iPatch;     ///< Patch number
    uint8_t field;      ///< Field identifier
    param_t value;      ///< New setting value
};

/// @brief Patch update data - An operator setting has changed
struct PatchOpChange {
    uint8_t iPatch;     ///< Patch number
    uint8_t iOp;        ///< OpParams number
    uint8_t field;      ///< Field identifier
    param_t value;      ///< New setting value
};

// Serialized data sizes
constexpr size_t opParamsChangeDataSize = Serialize::serializeHdrSize + 5;
constexpr size_t patchSettingChangeDataSize = Serialize::serializeHdrSize + 4;
constexpr size_t patchNameChangeDataSize = Serialize::serializeHdrSize + 1 + patchNameLen;
constexpr size_t patchChangeDataSize = Serialize::serializeHdrSize + 1 + patchSize;

/// @brief Update a Patch in the current PatchBank
/// @param change Updated patch data downloaded over USB
void mergePatchChange(const PatchChange& change);

/// @brief Update a Patch name in the current PatchBank
/// @param change Updated patch data downloaded over USB
void mergePatchChange(const PatchNameChange& change);

/// @brief Update a Patch setting in the current PatchBank
/// @param change Updated patch data downloaded over USB
void mergePatchChange(const PatchSettingChange& change);

/// @brief Update a Patch operator setting in the current PatchBank
/// @param change Updated patch data downloaded over USB
void mergePatchChange(const PatchOpChange& change);

} } // namespace Patches
