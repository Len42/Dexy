#pragma once

namespace Dexy {

/// @brief Patch definition and storage - Version 2
namespace Patches { namespace V2 {

/// @brief Envelope level scaling curve type
enum class ScalingCurve : signed char {
    NExp = -2, NLin = -1, None = 0, Lin = 1, Exp = 2
};

// TODO: LevelScalingParams should be in OpParams not EnvParams
/// @brief Envelope level scaling parameters
struct LevelScalingParams
{
    midiNote_t breakPoint = 60 * midiNoteSemitone;  ///< Left-right break point
    ScalingCurve curveLeft = ScalingCurve::None;    ///< Scaling curve to the left
    ScalingCurve curveRight = ScalingCurve::None;   ///< Scaling curve to the right
    param_t depthLeft = 0;                          ///< Scaling depth to the left
    param_t depthRight = 0;                         ///< Scaling depth to the right
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
};

/// @brief Number of patches in a Dexy::Patches::PatchBank
constexpr unsigned numPatches = 32;

/// @brief The set of patches that are stored in the module
/// @details There is one instance of PatchBank containing the current set of patches.
struct PatchBank
{
    /// @brief Collection of patches
    std::array<Patch, numPatches> patches;
};

// Sizes of serialized data objects
// These must be defined explicitly but the sizes are verified at compile time just in case.
// There are no variable-size data members in our serializable types.
// Note that these sizes are different from sizeof(x) because serialization packs differently.
constexpr size_t envParamsSize = 13 + 8;
constexpr size_t opParamsSize = 8 + envParamsSize;
constexpr size_t patchSize = patchNameLen + 3 + 6 * opParamsSize;
constexpr size_t patchBankSize = numPatches * patchSize;

} } } // namespace Patches::V2
