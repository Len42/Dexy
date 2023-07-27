namespace Dexy {

/// @brief The number of bits in a progress_t
static constexpr unsigned cbitsProgress = cbitsPhase;

/// @brief Maximum value of a progress_t value
static constexpr Envelope::progress_t max_progress_t =
    (Envelope::progress_t(1) << cbitsProgress) - 1;

/// @brief Helper for initializing expRateMap
/// @note This relies on math functions (e.g. std::exp()) being declared
/// constexpr, which they are in gcc but not in other compilers or the
/// C++20/23 standard. :(
static constexpr Envelope::rate_t expRateMapEntry(std::size_t index, std::size_t /*unused*/)
{
    // This function must map index 0 -> value 1
    // and max_param_t-1 should give an attack/release time of about 1 ms.
    // max_param_t is a special value indicating the stage should be as short as possible.
    double value;
    if (index == max_param_t) {
        value = max_progress_t + 1;
    } else {
        value = std::round(std::exp(index * 0.01) * 12.0243) - 11;
    }
    return Envelope::rate_t(value);
}

/// @brief Lookup table for exponential mapping of rate parameters
static constexpr DataTable<Envelope::rate_t, max_param_t+1, expRateMapEntry> expRateMap;

/// @brief Convert an envelope rate setting (param_t) to a rate_t
static constexpr Envelope::rate_t rateFromParam(param_t param)
{
    // TODO: Does this give an appropriate range of values for an envelope?
    if (param > max_param_t)
        param = max_param_t;
    return expRateMap[param];
}


/// @brief Convert an envelope's attack setting (EnvParams::attack) to a rate_t
/// @details Attack rates are somewhat faster than decay/release rates, in keeping with the DX7.
static constexpr Envelope::rate_t attackRateFromParam(param_t param)
{
     return rateFromParam(param) * 3 / 2;
}

/// @brief Convert an envelope's decay or release setting (EnvParams::decay) to a rate_t
static constexpr Envelope::rate_t decayRateFromParam(param_t param)
{
     return rateFromParam(param);
}

/// @brief Attack profile lookup table using interpolation with data
/// pre-calculated at compile time
/// @details The attack profile is quadratic not exponential, in keeping with
/// the DX7's envelopes.
/// Table size is based on the format of phase_t: 512 samples plus an extra
/// entry at the end, duplicating the first, to help with interpolation.
/// The table must map index 0 -> value 0 and index 512 -> value 65535.
using AttackTable = WaveTable<level_t, sizeLookupTable,
    [](std::size_t index, [[maybe_unused]] std::size_t numValues) {
        return level_t(std::round(index * index * 0.2499962));
    }>;

// TODO: AttackStartTable doc
using AttackStartTable = WaveTable<uint16_t, sizeLookupTable,
    [](std::size_t index, [[maybe_unused]] std::size_t numValues) {
        return uint16_t(std::round(std::sqrt((index * 128) / 0.2499962) * 64));
    }>;

/// @brief Helper for initializing expRateMap
/// @note This relies on math functions (e.g. std::exp()) being declared
/// constexpr, which they are in gcc but not in other compilers or the
/// C++20/23 standard. :(
static constexpr level_t decayTableEntry(std::size_t index, std::size_t numValues)
{
    // TODO: Tune these values for a good curve.
    double value;
    if (index == numValues-1) {
        value = 0;
    } else {
        value = std::round(std::exp((numValues - 1 - index) * 0.01) * 393.996) - 394;
    }
    return level_t(value);
}

/// @brief Decay/release profile lookup table using interpolation with data
/// pre-calculated at compile time
/// @details Table size is based on the format of phase_t: 512 samples plus an
/// extra entry at the end, duplicating the first, to help with interpolation.
/// The table must map index 0 -> value 65535 and index 512 -> value 0.
/// @note This relies on math functions (e.g. std::exp()) being declared
/// constexpr, which they are in gcc but not in other compilers or the
/// C++20/23 standard. :(
using DecayTable = WaveTable<level_t, sizeLookupTable, decayTableEntry>;

/// @brief Lookup table for reverse mapping level -> progress, used when starting
/// decay and release stages.
/// @details The table must map index 0 -> value 32768 and index 512 -> value 0.
/// @note This relies on std::log() being declared constexpr,
/// which it is in gcc but not in other compilers or the C++20 standard. :(
using DecayStartTable = WaveTable<uint16_t, sizeLookupTable,
    [](std::size_t index, [[maybe_unused]] std::size_t numValues) {
        return uint16_t(std::round(32768 - std::log((index * 128 + 394) / 393.996) * 100 * 64));
    }>;

IN_FLASH("Envelope")
void Envelope::init()
{
    AttackTable::init();
    DecayTable::init();
    DecayStartTable::init();
}

void Envelope::setParams(const Patches::EnvParams& params)
{
    // Convert input parameters from param_t to appropriate implementation values.
    // TODO: Keyboard (pitch) rate scaling - single number per op; see Complete DX7
    // params.delay represents a time but delay is a rate, so change it around
    delay = rateFromParam(max_param_t - params.delay);
    attack = attackRateFromParam(params.attack);
    decay = decayRateFromParam(params.decay);
    sustain = Operator::levelFromParam(params.sustain);
    release = decayRateFromParam(params.release);
    loop = params.loop;
}

void Envelope::gateStart()
{
    gateOn = true;
    // Start the envelope
    setStage<Stage::Delay>();
}

void Envelope::gateStop()
{
    if (getAndSet(gateOn, false)) {
        setStage<Stage::Release>();
    }
}

level_t Envelope::genNextOutput()
{
    // Note: Each doStageFunction() is responsible for incrementing progress.
    // That's because lookupInterpolate() takes care of it.
    (this->*doStageFunction)();
    return level;
}

void Envelope::stopEnvelope()
{
    setStage<Stage::Idle>();
}

template<Envelope::Stage stage>
void Envelope::setStage()
{
    doStageFunction = &Envelope::doStage<stage>;
    initStage<stage>();
}

template<>
void Envelope::initStage<Envelope::Stage::Idle>()
{
    progress = 0;
    increment = 0;
    level = 0;
}

template<>
void Envelope::doStage<Envelope::Stage::Idle>()
{
    // idling forever...
}

template<>
void Envelope::initStage<Envelope::Stage::Delay>()
{
    progress = 0;
    increment = delay;
    // level = 0; // Don't do this because it can make a click if the previous
                  // release is still running.
    // BUG: That means that the previous output level continues during the delay
    // stage. The fix for that is to make Delay do the same as Release, but then
    // two progress variables are needed - one to track the delay time and one to
    // run the release curve.
}

template<>
void Envelope::doStage<Envelope::Stage::Delay>()
{
    if (progress >= max_progress_t) {
        setStage<Stage::Attack>();
    } else {
        // delaying...
        progress += increment;
    }
}

template<>
void Envelope::initStage<Envelope::Stage::Attack>()
{
    // Initialize progress so we use the tail end of the attack curve instead
    // of starting at the beginning, to avoid a click when starting a note while
    // the previous note envelope is running.
    progress_t index = progress_t(level) << 8;
    progress = 512 * AttackStartTable::lookupInterpolate(&index, 0);
    //level starts at its current value and goes up from there
    increment = attack;
}

template<>
void Envelope::doStage<Envelope::Stage::Attack>()
{
    if (progress >= max_progress_t) {
        // Must set level = max_level_t here so it works when attack time is zero
        level = max_level_t;
        setStage<Stage::Decay>();
    } else {
        // lookupInterpolate() takes care of incrementing progress.
        level = AttackTable::lookupInterpolate(&progress, increment);
    }
}

template<>
void Envelope::initStage<Envelope::Stage::Decay>()
{
    // Initialize progress so we use the tail end of the decay curve instead
    // of starting at the beginning, to give a gentler decay.
    progress_t index = progress_t(level - sustain) << 8;
    progress = 512 * DecayStartTable::lookupInterpolate(&index, 0);
    increment = decay;
    //level starts at its current value
}

template<>
void Envelope::doStage<Envelope::Stage::Decay>()
{
    if (level <= sustain || progress >= max_progress_t) {
        if (loop) {
            // loop mode - no sustain
            setStage<Stage::Release>();
        } else {
            setStage<Stage::Sustain>();
        }
    } else {
        // lookupInterpolate() takes care of incrementing progress.
        level = DecayTable::lookupInterpolate(&progress, increment);
        if (max_level_t - level >= sustain)
            level += sustain;
        else
            level = max_level_t;
    }
}

template<>
void Envelope::initStage<Envelope::Stage::Sustain>()
{
    progress = 0;
    increment = 0;
    level = sustain;
}

template<>
void Envelope::doStage<Envelope::Stage::Sustain>()
{
    // just sustaining...
    level = sustain; // in case it's changed by live updating
}

template<>
void Envelope::initStage<Envelope::Stage::Release>()
{
    // Initialize progress so we use the tail end of the decay curve instead
    // of starting at the beginning, to give a gentler decay.
    progress_t index = progress_t(level) << 8;
    progress = 512 * DecayStartTable::lookupInterpolate(&index, 0);
    //level starts at its current value and goes down from there
    increment = release;
}

template<>
void Envelope::doStage<Envelope::Stage::Release>()
{
    if (level == 0 || progress >= max_progress_t) {
        if (loop && gateOn) {
            // loop mode - restart the envelope
            setStage<Stage::Delay>();
        } else {
            stopEnvelope();
        }
    } else {
        // lookupInterpolate() takes care of incrementing progress.
        level = DecayTable::lookupInterpolate(&progress, increment);
    }
}

}
