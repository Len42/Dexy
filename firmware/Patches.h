#pragma once

namespace Dexy {

/// @brief Patch definition and storage
namespace Patches {

/// @brief Envelope level scaling curve type
enum class ScalingCurve : signed char {
    NExp = -2, NLin = -1, None = 0, Lin = 1, Exp = 2
};

/// @brief Envelope level scaling parameters
struct LevelScalingParams
{
    midiNote_t breakPoint = 60 * midiNoteSemitone;  ///< Left-right break point
    ScalingCurve curveLeft = ScalingCurve::None;    ///< Scaling curve to the left
    ScalingCurve curveRight = ScalingCurve::None;   ///< Scaling curve to the right
    param_t depthLeft = 0;                          ///< Scaling depth to the left
    param_t depthRight = 0;                         ///< Scaling depth to the right
    constexpr bool isValid() const;                 ///< Does this contain valid settings?
};

/// @brief Envelope parameters
struct EnvParams
{
    param_t delay = 0;                  ///< Delay time
    param_t attack = 0;                 ///< Attack rate
    param_t decay = 0;                  ///< Decay rate
    param_t sustain = max_param_t;      ///< Sustain level
    param_t release = 0;                ///< Release rate
    param_t rateScaling = 0;            ///< Rate scaling by pitch
    LevelScalingParams levelScaling;    ///< Level scaling parameters
    bool loop = false;                  ///< If true, the envelope repeats indefinitely
    constexpr bool isValid() const;     ///< Does this contain valid settings?
};

/// @brief Operator parameters
struct OpParams
{
    bool fixedFreq = false;             ///< Frequency is either fixed or set by pitch CV
    uint16_t noteOrFreq = freqRatio1;   ///< freqRatio_t or midiNote_t depending on fixedFreq
    param_t outputLevel = max_param_t;  ///< Output level nominal maximum
    bool useEnvelope = true;            ///< Is output level modulated by the envelope?
    EnvParams env;                      ///< Envelope parameters
    param_t ampModSens = 0;             ///< Operator sensitivity to amplitude modulation
    constexpr bool isValid() const;     ///< Does this contain valid settings?
};

/// @brief Length of a patch's name
static constexpr unsigned patchNameLen = 16;

/// @brief Patch name is a fixed-size array of characters (not 0-terminated)
using patchName_t = std::array<char, patchNameLen>;

/// @brief Parameters of a single patch
struct Patch
{
    std::array<OpParams, numOperators> opParams;
    uint8_t algorithm = 0;
    param_t feedbackAmount = max_param_t;
    patchName_t name = {' '};
    constexpr bool isValid() const;     ///< Does this contain valid settings?
};

/// @brief The set of patches that are stored in the module
/// @details There is one instance of PatchBank containing the current set of patches.
struct PatchBank
{
    /// @brief Collection of patches
    std::array<Patch, numPatches> patches;

    /// @brief Are all the patches valid?
    constexpr bool isValid() const;
};

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
