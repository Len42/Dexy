namespace Dexy { namespace Synth {

/// @brief Index of the currently-playing patch in the patchbank
static unsigned patchIndex = 0;

/// @brief Name of the currently-playing patch
static Patches::patchName_t patchName = {' '};

/// @brief Array of Operator that make the sound!
static Operator operators[numOperators];

/// @brief The Algorithm that controls how the operators are combined
static Algorithm algorithm = algorithms[0];

/// @brief Feedback amount to use
static param_t feedbackAmount = max_param_t;

/// @brief The current timbre (amplitude) modulation value
/// @details This is set based on a CV input.
static output_t timbreMod = 0;

#ifdef DEBUG_TEST_LFO
/// @brief Operator to use as an LFO (for debugging only)
static Operator opLfo;
#endif

// Forward
static void loadPatchDeferred(Defer::UseCritSec, unsigned index);
static void loadPatchImpl(unsigned index);
static void initOperators();

void init()
{
    // Validate all algorithm definitions (at compile time)
    // because it's easy to get them wrong
    static_assert(numAlgorithms == std::size(algorithms));
    static_assert(std::ranges::all_of(algorithms,
        [](const Algorithm& algo){return algo.isValid();}));

#ifdef DEBUG_DUMP_ALGORITHMS
    for (int i = 0; i < numAlgorithms; ++i) {
        printf("algorithm %d: ", i+1);
        if (!algorithms[i].isValid())
            printf("ERROR ");
        algorithms[i].dump();
    }
#endif

    SineWave::init();

    Envelope::init();

    loadPatchImpl(initialPatch);

    initOperators();
}

static void initOperators()
{
#ifdef DEBUG_TEST_LFO
    // Set up an LFO so we have a source to test operator amplitude modulation
    opLfo.setOpParams(Patches::OpParams{
        .fixedFreq = true,
        .noteOrFreq = uint16_t(-36 * midiNoteSemitone), // approx. 1 Hz
        .outputLevel = max_param_t,
        .useEnvelope = false,
        .env(),
        .ampModSens = 0
    });
#endif
}

void loadPatch(unsigned i)
{
    Defer::call<loadPatchDeferred>(i);
}

/// @brief Helper function called from loadPatch() via Defer
/// @details The arguments force Defer to use a mutex because it can be a cross-core call.
/// @param index Patch number
static void loadPatchDeferred(Defer::UseCritSec /*unused*/, unsigned index)
{
    loadPatchImpl(index);
}

/// @brief Load the given patch from the current patchbank.
/// @details This function sets the Synth parameters based on the given patch.
/// @param index Patch number
static void loadPatchImpl(unsigned index)
{
    if (index < numPatches) {
        patchIndex = index;
        const Patches::Patch& patch = Patches::getPatch(index);
        algorithm = algorithms[patch.algorithm];
        feedbackAmount = patch.feedbackAmount;
        for (unsigned i = 0; i < numOperators; ++i) {
            operators[i].setOpParams(patch.opParams[i]);
            operators[i].resetWave();
            // don't reset the envelope becuase that messes up live updating
        }
        patchName = patch.name;
    }
}

unsigned getCurrentPatchNum()
{
    return patchIndex;
}

const Patches::patchName_t& getCurrentPatchName()
{
    return patchName;
}

void setNotePitch(phase_t pitch)
{
    // TODO: crit sec?
    for (auto&& op : operators) {
        op.setNotePitch(pitch);
    }
}

void setTimbreMod(output_t value)
{
    // TODO: crit sec?
    timbreMod = value;
}

void gateStart()
{
    for (auto&& op : operators) {
        op.gateStart();
    }
    // Notify the UI task so it can draw some graphics
    UI::UITask::onGateStart();
}

void gateStop()
{
    for (auto&& op : operators) {
        op.gateStop();
    }
}

__attribute__((__always_inline__))
inline output_t genNextOutput()
{
#ifdef DEBUG_TEST_LFO
    // TEST: Iterate an LFO to generate timbre modulation for testing
    setTimbreMod(opLfo.genNextOutput(0, 0));
#endif

    // Check if a new patch was requested
    Defer::checkRun<loadPatchDeferred>();

    // Call genNextOutput() on each operator, handling modulation and feedback,
    // based on the currently-selected algorithm.
    int32_t outputTotal = 0;
    output_t freqModPrev = 0;
    output_t freqModSaved = 0;
    static int32_t feedback0 = 0;
    static int32_t feedback1 = 0;
    for (unsigned i = 0; i < numOperators; ++i) {
        Operator& op = operators[i];
        const AlgoOp& algoOp = algorithm.ops[i];
        // Set the appropriate modulation for this operator
        output_t freqMod;
        switch (algoOp.mod) {
            case UseMod::prev:
                freqMod = freqModPrev;
                break;
            case UseMod::saved:
                freqMod = freqModSaved;
                break;
            case UseMod::fb:
                // Feedback is the average of the two previous values, attenuated by feedbackAmount
                // TODO: Scope this! Should we really have to divide by 16?
                freqMod = output_t((int32_t(feedbackAmount) * ((feedback0 + feedback1) / 2)) / (1024*16));
                break;
            case UseMod::none:
            default:
                freqMod = 0;
                break;
        }
        // Calculate this operator's output
        output_t outputOp = op.genNextOutput(freqMod, timbreMod);
        // Save the operator's output as either an audio output or a modulator
        if (algoOp.isOutput) {
            outputTotal += outputOp;
        } else {
            freqModPrev = outputOp;
            switch (algoOp.saveMod) {
                case SaveMod::set:
                    freqModSaved = outputOp;
                    break;
                case SaveMod::add:
                    freqModSaved += outputOp;
                    break;
                default:
                case SaveMod::none:
                    break;
            }
        }
        if (algoOp.setFb) {
            // Current and previous outputs are saved for averaged feedback
            feedback1 = feedback0;
            feedback0 = outputOp;
        }
    }

    return output_t(outputTotal / algorithm.numOutputs);
}

} } // namespace Synth
