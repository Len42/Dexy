// Envelope - Envelope generation

#pragma once

namespace Dexy {

/// @brief Envelope generator
///
/// This is a DADSR envelope generator. It has a few similarities with the envelopes
/// on the DX7 but is basically different (simpler).
///
/// Note that Attack, Decay, and Release are rates, not times. (This matches the DX7
/// behaviour.)
///
/// DX7 envelopes do not have an explicit Delay stage, but they do have a hidden
/// feature that two successive levels that are almost equal act as a delay.
/// The main use for that feature is for delayed attack so this class implements
/// a Delay stage instead.
///
/// Rate and delay time settings are exponential. For example, a Delay setting
/// of 50% will result in a delay time that is much less than 50% of maximum.
///
/// Attack rates are somewhat faster than decay/release rates. This reflects DX7
/// behaviour.
///
/// Notes
/// -----
/// https://mrgsynth.tlbflush.org/notes/post/dx7eg2/ shows (I think):
/// - level is exponential
/// - release from less-than-max level does the "tail end" of the exponential
///   curve, not the start of the curve
///
/// There is no global pitch envelope like the DX7 has. An external envelope can be
/// patched into the pitch mod input.
///
/// @todo
/// Max level (after attack) has been considered but not implemented.
/// It's redundant with operator output level, and is a bit of a pain to implement,
/// but maybe it should be supported for ease of porting DX7 patches.
/// - Max & sustain levels _should_ be scaled by output level (see Complete DX7)
/// - do rates change too??
///
/// @todo Tune parameters to match the ones given in the article (or a better source)
/// (sort of done but still not sure they're good overall)
///
/// @todo EG rate & level scaling
class Envelope
{
public:
    /// @brief Initialization - must be called at startup
    static void init();

    /// @brief Set envelope parameters
    void setParams(const Patches::EnvParams& params);

    /// @brief Gate start signal has been received - Start the envelope running
    void gateStart();

    /// @brief Gate stop signal has been received - Start the envelope's release stage
    void gateStop();

    /// @brief Generate the next envelope value
    level_t genNextOutput();

    /// @brief Stop the envelope and return to idle state
    void stopEnvelope();

    /// @brief Current position in the current envelope stage
    /// @details Actually the same as Dexy::phase_t.
    using progress_t = phase_t;

    /// @brief Rate at which the envelope is changing
    /// @details Actually the same as Dexy::phase_t.
    /// @todo Put this in Defs.h and use it wherever phase_t is actually an increment
    using rate_t = phase_t;

private:
    // Envelope settings - based on EnvParams but stored as implementation-
    // friendly types
    rate_t delay = 0;   ///< Delay - logically a time but implemented as a rate
    rate_t attack = 0;  ///< Attack rate
    rate_t decay = 0;   ///< Decay rate
    level_t sustain = max_level_t;  ///< Sustain level
    rate_t release = 0; ///< Release rate
    bool loop = false;  ///< If true, the envelope loops continuously while gate is on

    // Current status of the envelope
    progress_t progress = 0;    ///< Current position in the current stage
    progress_t delayProgress = 0;    ///< Current position in the delay stage
    rate_t increment = 0;       ///< Amount to increment progress at each step
    level_t level = 0;          ///< Current envelope level
    bool gateOn = false;        ///< Is the gate on? - used for looping

    /// @brief Envelope stages
    /// @details Each envelope stage has an init function and a do function
    enum class Stage { Idle, Delay, Attack, Decay, Sustain, Release };

    /// @brief Set the envelope stage
    template<Stage stage> void setStage();

    /// @brief Member function pointer that represents the current Stage
    /// @details This is called periodically to update the envelope state.
    void (Envelope::* doStageFunction)() = &Envelope::doNothing;

    /// @brief Template for each stage's initialization function, which is
    /// called when the envelope enters the given Stage.
    /// @tparam stage 
    /// @details This is specialized for each Stage. The non-specialized
    /// version is not defined.
    template<Stage stage> void initStage();

    /// @brief Template for each stage's "do" function, which is called
    /// periodically to update the envelope state.
    /// @tparam stage 
    /// @details This is specialized for each Stage. The non-specialized
    /// version is not defined.
    ///
    /// doStage() is responsible for incrementing progress. That's because
    /// WaveTable::lookupInterpolate() takes care of it.
    /// @see doStageFunction()
    template<Stage stage> void doStage();

    /// @brief Placeholder to initialize doStageFunction
    void doNothing() { }
};

}
