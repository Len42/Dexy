#pragma once

namespace Dexy { namespace Patches {

/// @brief Compile-time function to make a simple but usable Patch
/// @return Patch
consteval Patch makeDefaultPatch()
{
    Patch patch;
    patch.name = strToArray("default         ");
    patch.algorithm = 15;
    patch.feedbackAmount = max_param_t;
    for (auto&& params : patch.opParams) {
        params = OpParams{
                    .fixedFreq = false,
                    .noteOrFreq = uint16_t(2 * freqRatio1),
                    .outputLevel = 700,
                            // 850: dexed 99
                            // 600: dexed 75-76
                            // 512: dexed 71-72
                    .useEnvelope = true,
                    .env{
                        .delay = 0,
                        .attack = 930,
                        .decay = 500,
                        .sustain = 700,
                        .release = 400,
                        .loop = false
                    },
                    .ampModSens = max_param_t //0
                };
    }
    // Different settings for the last carrier operator
    patch.opParams[numOperators - 1] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(freqRatio1),
                .outputLevel = max_param_t,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 930,
                    .decay = 400,
                    .sustain = 930,
                    .release = 400,
                    .loop = false
                },
                .ampModSens = 0 // max_param_t
    };
    return patch;
}

/// @brief Compile-time function to make a bell-like Patch
/// @return Patch
consteval Patch makeBellPatch()
{
    Patch patch;
    patch.name = strToArray("bells           ");
    patch.algorithm = 4;
    patch.feedbackAmount = max_param_t;
    // carrier
    patch.opParams[5] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(1.01 * freqRatio1),
                .outputLevel = 972,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 1000,
                    .decay = 300,
                    .sustain = 0,
                    .release = 300,
                    .loop = false
                },
                .ampModSens = 0
    };
    // mod
    patch.opParams[4] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(3.51 * freqRatio1),
                .outputLevel = 700,//800,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 1020,
                    .decay = 120,
                    .sustain = 0,
                    .release = 200,
                    .loop = false
                },
                .ampModSens = 0
    };
    // carrier
    patch.opParams[3] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(0.99 * freqRatio1),
                .outputLevel = max_param_t,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 1000,
                    .decay = 300,
                    .sustain = 0,
                    .release = 300,
                    .loop = false
                },
                .ampModSens = 0
    };
    // mod
    patch.opParams[2] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(3.49 * freqRatio1),
                .outputLevel = 670,//770,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 1020,
                    .decay = 120,
                    .sustain = 0,
                    .release = 200,
                    .loop = false
                },
                .ampModSens = 0
    };
    // carrier
    patch.opParams[1] = OpParams{
                .fixedFreq = true,
                .noteOrFreq = uint16_t(64 * midiNoteSemitone),
                .outputLevel = max_param_t,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 700,
                    .decay = 500,
                    .sustain = 0,
                    .release = 600,
                    .loop = false
                },
                .ampModSens = 0
    };
    // mod, feedback
    patch.opParams[0] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(1.99 * freqRatio1),
                .outputLevel = 800,//900,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 1020,
                    .decay = 800,//910,
                    .sustain = 0,
                    .release = 910,
                    .loop = false
                },
                .ampModSens = 0
    };
    return patch;
}

/// @brief Compile-time function to make a simple Patch for testing and tuning
/// @return Patch
consteval Patch makeTestPatch()
{
    Patch patch;
    patch.name = strToArray("test            ");
    patch.algorithm = 31;
    patch.feedbackAmount = 0; //max_param_t;
    for (auto&& params : patch.opParams) {
        params = OpParams{
                    .fixedFreq = false,
                    .noteOrFreq = uint16_t(freqRatio1),
                    .outputLevel = 0,
                    .useEnvelope = true,
                    .env{
                        .delay = 0,
                        .attack = 930,
                        .decay = 400,
                        .sustain = 930,
                        .release = 400,
                        .loop = false
                    },
                    .ampModSens = 0
                };
    }
    // Different settings for the last carrier operator
    patch.opParams[0] = OpParams{
                .fixedFreq = false,
                .noteOrFreq = uint16_t(freqRatio1),
                .outputLevel = max_param_t,
                .useEnvelope = true,
                .env{
                    .delay = 0,
                    .attack = 900,
                    .decay = 0,
                    .sustain = max_param_t,
                    .release = 400,
                    .loop = false
                },
                .ampModSens = 0
    };
    return patch;
}

/// @brief Compile-time function to make a default PatchBank full of patches
/// @return PatchBank
consteval PatchBank makeDefaultPatchBank()
{
    PatchBank patchBank;
    // Initialize with a bunch of default patches
    // TODO: Load some patches that are more interesting
    for (auto&& patch : patchBank.patches) {
        patch = makeDefaultPatch();
    }
    patchBank.patches[0] = makeBellPatch();
    patchBank.patches[1] = makeTestPatch();
    return patchBank;
}

} } // namespace Patches
