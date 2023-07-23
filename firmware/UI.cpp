namespace Dexy { namespace UI {

/// @brief User interface states
enum class State { SplashScreen, Idle, Sleep, Select, Message };

/// @brief Placeholder to initialize doStateFunction
static void doNothing() { }

/// @brief Function pointer that represents the current State
/// @details This is called periodically to handle input and update the UI state.
static auto doStateFunction = doNothing;

/// @brief Template for each state's initialization function, which is called
/// when entering the given State.
/// @details This is specialized for each State. The non-specialized version
/// is not defined.
/// @tparam state
template<State state> void initState();

/// @brief Template for each state's "do" function, which is called peridically
/// to handle input and update the UI state.
/// @details This is specialized for each State. The non-specialized
/// version is not defined.
/// @tparam state 
template<State state> void doState();

// Timeouts for various states
static constexpr unsigned timeoutSplashScreen = 5000;   ///< Splash screen timeout (ms)
static constexpr unsigned timeoutIdle = 5000;           ///< Idle screen timeout (ms)
static constexpr unsigned timeoutSelect = 3000;         ///< Select mode timeout (ms)
static constexpr unsigned timeoutMessage = 3000;        ///< Message timeout (ms)

// Forward declarations
template<State state> static void setState();
static void setTimeout(uint32_t delayMs);
static bool checkTimeout();
static bool checkActivity();
static bool checkPatchesUpdated();
static void showCurrentPatch();
static void showSelectedPatch();
static void showPatchList(int iSelected);
static void showSelectedPatchCompact(int iSelected);
// Call showMessage("message") instead of setState<State::Message>()
template<size_t SIZE> static void showMessage(const char(&str)[SIZE]);
static void drawNoteStart();
static void drawNoteStop();
static void drawNoteUpdate();
static void onGateStartDeferred(Defer::UseCritSec /*unused*/, unsigned /*unused*/);
static void onPatchSelectedDeferred(unsigned /*unused*/);
static void onPatchBankUpdateDeferred(unsigned /*unused*/);

IN_FLASH("UI")
void UITask::init()
{
    setState<State::SplashScreen>();
}

IN_FLASH("UI")
void UITask::execute()
{
    doStateFunction();
}

void UITask::onGateStart()
{
    // Make a deferred call to the implementation because this is a cross-core call.
    Defer::call<onGateStartDeferred>();
}

/// @brief Called from UITask::onGateStart() via Defer.
/// @details The arguments force Defer to use a mutex because it can be a cross-core call.
IN_FLASH("UI")
static void onGateStartDeferred(Defer::UseCritSec /*unused*/, unsigned /*unused*/)
{
    drawNoteStart();
}

IN_FLASH("UI")
void UITask::onPatchSelected()
{
    Defer::call<onPatchSelectedDeferred>();
}

/// @brief Called from UITask::onPatchSelected() via Defer.
IN_FLASH("UI")
static void onPatchSelectedDeferred(unsigned /*unused*/)
{
    // A different patch has been selected (but not by this Task).
    // Idle state will display the selected patch.
    setState<State::Idle>();
}

IN_FLASH("UI")
void UITask::onPatchBankUpdate()
{
    Defer::call<onPatchBankUpdateDeferred>();
}

/// @brief Called from UITask::onPatchBankUpdate() via Defer.
IN_FLASH("UI")
static void onPatchBankUpdateDeferred(unsigned /*unused*/)
{
    showMessage("Patches Updated");
}

/// @brief Set the UI to a different State
template<State state>
IN_FLASH("UI")
static void setState()
{
    doStateFunction = doState<state>;
    initState<state>();
}

/// @brief Show the startup splash screen
template<>
IN_FLASH("UI")
void initState<State::SplashScreen>()
{
    Display::showSplashScreen();
    setTimeout(timeoutSplashScreen);
}

/// @brief Display the splash screen until something happens or timeout
template<>
IN_FLASH("UI")
void doState<State::SplashScreen>()
{
    if (checkPatchesUpdated()) {
        ; // nothing else to do
    } else if (checkTimeout() || checkActivity()) {
        setState<State::Idle>();
    }
}

/// @brief When entering Idle state, display the patch that is currently playing
template<>
IN_FLASH("UI")
void initState<State::Idle>()
{
    showCurrentPatch();
    setTimeout(timeoutIdle);
}

/// @brief Display the idle message until something happens or timeout
template<>
IN_FLASH("UI")
void doState<State::Idle>()
{
    if (checkPatchesUpdated()) {
        ; // nothing else to do
    } else if (checkActivity()) {
        setState<State::Select>();
    } else if (checkTimeout()) {
        setState<State::Sleep>();
    }
}

/// @brief When entering Sleep state, clear the display
template<>
IN_FLASH("UI")
void initState<State::Sleep>()
{
    // After being idle for a while, blank the screen to prevent burn-in.
    // While in this state, animations are displayed when notes are played.
    Display::clear();
    Defer::clearPending<onGateStartDeferred>();
    drawNoteStop();
}

/// @brief While in Sleep state, display animations when notes are played
template<>
IN_FLASH("UI")
void doState<State::Sleep>()
{
    if (checkPatchesUpdated()) {
        ; // nothing else to do
    } else if (checkActivity()) {
        setState<State::Select>();
    } else if (Defer::checkRun<onGateStartDeferred>()) {
        ; // nothing else to do
    } else {
        drawNoteUpdate();
    }
}

/// @brief The index of the currently selected item
static int iSelection = 0;

/// @brief Initialize Select state, which selects a patch using the rotary encoder
template<>
IN_FLASH("UI")
void initState<State::Select>()
{
    iSelection = Synth::getCurrentPatchNum();
    showSelectedPatch();
    setTimeout(timeoutSelect);
}

/// @brief While in Select state, track encoder movement and change the selected
/// patch, until the encoder button is pressed or timeout
template<>
IN_FLASH("UI")
void doState<State::Select>()
{
    if (checkTimeout()) {
        setState<State::Idle>();
    } else {
        bool fButtonPressed = Encoder::getInstance().checkSwitch();
        int selectionChange = Encoder::getInstance().getChange();
        if (fButtonPressed) {
            Synth::loadPatch(iSelection);
            sleep_ms(1); // kludge: give time for patch to load asynchronously
            setState<State::Idle>();
        } else if (selectionChange) {
            iSelection += selectionChange;
            iSelection = std::clamp(iSelection, 0, int(numPatches)-1);
            showSelectedPatch();
            setTimeout(timeoutSelect);
        }
    }
}

/// @brief Enter Message state
/// @details This state should be entered by calling showMessage(), not by calling
/// setState() directly.
template<>
IN_FLASH("UI")
void initState<State::Message>()
{
    setTimeout(timeoutMessage);
}

/// @brief Display the current message until something happens or timeout
template<>
IN_FLASH("UI")
void doState<State::Message>()
{
    if (checkActivity()) {
        setState<State::Select>();
    } else if (checkTimeout()) {
        setState<State::Idle>();
    }
}

/// @brief When the current timeout period will expire
static absolute_time_t timeout = from_us_since_boot_constexpr(0);

/// @brief Set the timeout expiration time
/// @param delayMs Milliseconds from now
IN_FLASH("UI")
static void setTimeout(uint32_t delayMs)
{
    timeout = make_timeout_time_ms(delayMs);
}

/// @brief Check if the current timeout period has expired
/// @return Yes or no
IN_FLASH("UI")
static bool checkTimeout()
{
    return timeIsReached(get_absolute_time(), timeout);
}

/// @brief Check if the rotary encoder has been turned or pressed
/// @details This clears the encoder state.
/// @return Yes or no
IN_FLASH("UI")
static bool checkActivity()
{
    return Encoder::getInstance().checkSwitch() || Encoder::getInstance().getChange();
}

/// @brief Check if a patchbank-updated notification has been received
/// @return Yes or no
IN_FLASH("UI")
static bool checkPatchesUpdated()
{
    return Defer::checkRun<onPatchBankUpdateDeferred>()
        || Defer::checkRun<onPatchSelectedDeferred>();
}

/// @brief Display the name of the patch that is currently playing
IN_FLASH("UI")
static void showCurrentPatch()
{
    Display::startDrawing();
    static constexpr char strTitle[] = "Now Playing:";
    auto& patchName = Synth::getCurrentPatchName();
    unsigned height = Display::getTextHeight(strTitle, 0, false)
                    + Display::getTextHeight(patchName, 0, true);
    unsigned y = (Display::screenHeight() - height) / 2;
    y = Display::drawText(strTitle, 0, y, false);
    Display::drawText(patchName, 0, y, true);
    Display::endDrawing();
}

/// @brief In Select mode, display the name of the patch that is currently selected
/// @details If compiled for a larger OLED display, show a scrolling list of patches.
/// On a smaller display, just show a single patch with a number.
IN_FLASH("UI")
static void showSelectedPatch()
{
    if constexpr (Display::isLargeDisplay) {
        showPatchList(iSelection);
    } else {
        showSelectedPatchCompact(iSelection);
    }        
}

/// @brief Display (part of) the list of patches in the patch bank, with an indicator
/// for the one that is currently selected
/// @details Helper for showSelectedPatch()
/// @param iSelected 
IN_FLASH("UI")
static void showPatchList(int iSelected)
{
    Display::showList(iSelected, [](int i) {
        return (i < int(numPatches))
            ? std::tuple(Patches::getPatch(i).name.data(), Patches::patchNameLen)
            : std::tuple(nullptr, 0u);
    });
}

/// @brief Display the name of the patch that is currently selected
/// @details Helper for showSelectedPatch()
/// @param iSelected 
IN_FLASH("UI")
static void showSelectedPatchCompact(int iSelected)
{
    Display::startDrawing();
    static constexpr char strTitle[] = "Select:";
    auto& patchName = Patches::getPatch(iSelected).name;
    unsigned height = 2 * Display::charHeight(); // 2 lines only, no wrapping
    unsigned x = 0;
    unsigned y = (Display::screenHeight() - height) / 2;
    y = Display::drawText(strTitle, 0, y, false);
    ++iSelected; // display numbering starts at 1
    if (iSelected >= 10) {
        Display::drawText(char('0' + iSelected/10), x, y);
        x += Display::charWidth();
    }
    Display::drawText(char('0' + iSelected%10), x, y);
    x += Display::charWidth() * 3/2;
    Display::drawText(patchName, x, y, true);
    Display::endDrawing();
}

/// @brief Display a message and enter Message state
/// @tparam SIZE String length (including \0), deduced from the str argument
/// @param str Null-terminated string constant
template<size_t SIZE>
static void showMessage(const char(&str)[SIZE])
{
    Display::startDrawing();
    unsigned y = 0;
    Display::drawText(str, 0, y, true);
    Display::endDrawing();
    setState<State::Message>();
}

static bool fDrawingNote = false;           ///< Is note animation running?
static unsigned xNote = 0;                  ///< X coord for note animation
static unsigned yNote = 0;                  ///< Y coord for note animation
static unsigned rNote = 0;                  ///< Radius for note animation
static constexpr unsigned rNoteInit = 2;    ///< Initial radius for note animation
static constexpr unsigned rNoteMax = 32;    ///< Max radius for note animation

/// @brief Start the animation for a note playing
IN_FLASH("UI")
static void drawNoteStart()
{
    // Use the raw ADC input as an approximation of the current note pitch
    // because it's easy to get and it has a good range.
    AdcInput::adcResult_t note = AdcInput::getCurrentValue<Gpio::adcInputPitch>();
    xNote = note / 32;
    yNote = note % 32;
    rNote = rNoteInit;
    fDrawingNote = true;
    drawNoteUpdate();
}

/// @brief Stop the animation for a note playing
IN_FLASH("UI")
static void drawNoteStop()
{
    fDrawingNote = false;
}

/// @brief Draw the animation for a note playing
IN_FLASH("UI")
static void drawNoteUpdate()
{
    if (fDrawingNote) {
        Display::startDrawing();
        if (rNote <= rNoteMax) {
            Display::drawCircle(xNote, yNote, rNote);
            rNote = rNote * 5 / 4 + 2;
        } else {
            drawNoteStop();
        }
        Display::endDrawing();
    }
}

} } // namespace UI
