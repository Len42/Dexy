namespace Dexy { namespace SerialIO {

/// @brief List of the commands received over the serial port
/// @see https://en.wikipedia.org/wiki/X_macro
#define FOR_EACH_COMMAND(DO) \
    DO(None, ) \
    DO(Version, vers) \
    DO(Upload, upld) \
    DO(Download, dnld) \
    DO(UpdPatch, upd1) \
    DO(UpdName, upd2) \
    DO(UpdSetting, upd3) \
    DO(UpdOperator, upd4) \
    DO(SelPatch, play) \
    DO(Boot, boot) \
    DO(BootLoad, btld) \
    DO(Invalid, )

/// @brief IDs of commands received over the serial port
enum class Command {
#define DECLARE_SERIAL_COMMAND(name, ...) name,
    FOR_EACH_COMMAND(DECLARE_SERIAL_COMMAND)
};

/// @brief Commands are received as 4-char strings
static constexpr unsigned commandSize = 4;

/// @brief Command strings
#define DEFINE_COMMAND_NAME(name, cmd, ...) static constexpr std::string_view command##name = #cmd##sv;
FOR_EACH_COMMAND(DEFINE_COMMAND_NAME)

/// @brief Template for command handlers
/// @tparam command 
/// @details This is specialized for each Command. The non-specialized version
/// is not defined.
template<Command command> static void doCommand();

/// @brief Input/output data is stored in a buffer
static inline Patches::SerializedPatchBank dataBuf;

// Forward declarations
static Command readCommand();
static int serialGetCharWait();
static int serialGetCharNoWait();
static bool serialReadData(auto* pbuf);
static bool serialReadData(auto* pbuf, size_t count);
void serialDrainInput();
static int serialWriteData(const auto& buf);
static void serialWriteLine(const char* str);
static void serialWriteAck();
// Forward-declare all the specializations of doCommand()
#define DECLARE_COMMAND_HANDLER(name, ...) template<> void doCommand<Command::name>();
FOR_EACH_COMMAND(DECLARE_COMMAND_HANDLER)

IN_FLASH("SerialIO")
void SerialIOTask::init()
{
    // get rid of bogus \0 on startup
    serialDrainInput();
}

IN_FLASH("SerialIO")
void SerialIOTask::execute()
{
    switch (readCommand()) {
        // Command dispatch - Handler functions are defined below
#define HANDLE_COMMAND(name, ...) case Command::name: doCommand<Command::name>(); break;
        FOR_EACH_COMMAND(HANDLE_COMMAND)
        default:
            doCommand<Command::Invalid>();
            break;
    }
}

/// @brief Read an incoming command
/// @return Command
IN_FLASH("SerialIO")
static Command readCommand()
{
    // First, check if there's any input using a non-blocking read.
    int ch;
    if ((ch = serialGetCharNoWait()) < 0) {
        // nothing to do right now
        return Command::None;
    } else {
        // A byte was read. Now read the rest of the command.
        char buf[commandSize];
        std::string_view bufSv(buf, commandSize);
        buf[0] = char(ch);
        for (auto& chBuf : std::ranges::drop_view(buf, 1)) {
            ch = serialGetCharWait();
            if (ch < 0) {
                dputs("SerialIO: ERROR: timeout/error");
                return Command::Invalid;
            }
            chBuf = char(ch);
        }
        // Find which command was read.
#define MATCH_COMMAND(name, ...) if (bufSv == command##name) return Command::name; else
        FOR_EACH_COMMAND(MATCH_COMMAND)
        {
            dputs("SerialIO: ERROR: invalid command");
            return Command::Invalid;
        }
    }
}

/// @brief Command::None does nothing
template<>
IN_FLASH("SerialIO")
void doCommand<Command::None>()
{
    // nothing to do
}

/// @brief Command::Version outputs the firmware version number
template<>
IN_FLASH("SerialIO")
void doCommand<Command::Version>()
{
    serialWriteLine(VersionInfo::getName());
}

/// @brief Command::Upload outputs the current patch bank in serialized format
template<>
IN_FLASH("SerialIO")
void doCommand<Command::Upload>()
{
    if (Patches::saveCurrentPatchBank(&dataBuf) != Patches::patchBankDataSize) {
        Error::set<Error::Err::BadPatchData>();
        return;
    }
    int c = serialWriteData(dataBuf);
    if (c != Patches::patchBankDataSize) {
        Error::set<Error::Err::SerialIO>();
        return;
    }
    Watchdog::petTheDog();
    dputs("Patch bank uploaded");
}

/// @brief Command::Download reads serialized patch bank data and replaces the
/// current patch bank
template<>
IN_FLASH("SerialIO")
void doCommand<Command::Download>()
{
    // Read patch bank data
    if (!serialReadData(&dataBuf)) {
        serialDrainInput();
        return;
    }
    // Load input data into the patch bank
    if (!Patches::loadCurrentPatchBank(dataBuf)) {
        return;
    }
    // Also save the serialized data to persistent flash memory
    Patches::saveInitialPatchData(dataBuf);
    serialWriteAck();
    dputs("Patch bank saved");
    // Notify Synth to reload the current patch...
    Synth::loadPatch(Synth::getCurrentPatchNum());
    // ...and notify UI to display a message.
    UI::UITask::onPatchBankUpdate();
}

/// @brief Command:UpdPatch receives and replaces an entire patch that has been updated
template<>
IN_FLASH("SerialIO")
void doCommand<Command::UpdPatch>()
{
    Patches::PatchChange change;
    if (!serialReadData(&dataBuf, Patches::patchChangeDataSize)) {
        // ? serialDrainInput();
        return;
    }
    if (!Serialize::readObject(dataBuf, &change)) {
        return;
    }
    Patches::mergePatchChange(change);
    serialWriteAck();
}

/// @brief Command:UpdName receives and updates a patch name that has been edited
template<>
IN_FLASH("SerialIO")
void doCommand<Command::UpdName>()
{
    Patches::PatchNameChange change;
    if (!serialReadData(&dataBuf, Patches::patchNameChangeDataSize)) {
        // ? serialDrainInput();
        return;
    }
    if (!Serialize::readObject(dataBuf, &change)) {
        return;
    }
    Patches::mergePatchChange(change);
    serialWriteAck();
}

/// @brief Command::UpdSetting receives and updates a patch setting that has been edited
template<>
IN_FLASH("SerialIO")
void doCommand<Command::UpdSetting>()
{
    Patches::PatchSettingChange change;
    if (!serialReadData(&dataBuf, Patches::patchSettingChangeDataSize)) {
        // ? serialDrainInput();
        return;
    }
    if (!Serialize::readObject(dataBuf, &change)) {
        return;
    }
    Patches::mergePatchChange(change);
    serialWriteAck();
}

/// @brief Command::UpdOperator receives and updates an operator setting that has been edited
template<>
IN_FLASH("SerialIO")
void doCommand<Command::UpdOperator>()
{
    Patches::PatchOpChange change;
    if (!serialReadData(&dataBuf, Patches::opParamsChangeDataSize)) {
        // ? serialDrainInput();
        return;
    }
    if (!Serialize::readObject(dataBuf, &change)) {
        return;
    }
    Patches::mergePatchChange(change);
    serialWriteAck();
}

/// @brief Command::SelPatch selects a given patch
template<>
IN_FLASH("SerialIO")
void doCommand<Command::SelPatch>()
{
    uint8_t iPatch;
    if (!serialReadData(&dataBuf, Serialize::serializeHdrSize + sizeof(iPatch))) {
        // ? serialDrainInput();
        return;
    }
    if (!Serialize::readObject(dataBuf, &iPatch)) {
        return;
    }
    Synth::loadPatch(iPatch);
    serialWriteAck();
    UI::UITask::onPatchSelected();
}

/// @brief Command::Boot reboots the microcontroller
template<>
IN_FLASH("SerialIO")
void doCommand<Command::Boot>()
{
    // Reboots - does not return
    dputs("Restarting...");
    watchdog_reboot(0, 0, 0);
    __builtin_unreachable();
}

/// @brief Command::BootLoad reboots the microcontroller in bootloader mode
template<>
IN_FLASH("SerialIO")
void doCommand<Command::BootLoad>()
{
    // Reboots into BOOTSEL mode - does not return
    dputs("Restarting in USB boot mode...");
    constexpr unsigned pinLedMask =
        (Gpio::pinLed == Gpio::pinNone) ? 0 : (1 << Gpio::pinLed);
    reset_usb_boot(pinLedMask, 0);
    __builtin_unreachable();
}

/// @brief Handle an invalid received command
template<>
IN_FLASH("SerialIO")
void doCommand<Command::Invalid>()
{
    serialDrainInput();
    Error::set<Error::Err::SerialIO>();
}

// Serial I/O helpers

/// @brief Serial read timeout (microseconds)
static constexpr unsigned readTimeout = 1'000'000;
// TODO: make this smaller
// 5000 should be enough to ensure proper reading while not triggering the watchdog

/// @brief Read a character from the serial input
/// @return The character read, or a negative value on timeout
/// @see readTimeout
static int serialGetCharWait()
{
    return getchar_timeout_us(readTimeout);
}

/// @brief Read a character from the serial input, only if there is a character
/// available to be read
/// @return The character read, or a negative value if none available
static int serialGetCharNoWait()
{
    return getchar_timeout_us(0);
}

/// @brief Read data into a buffer of known size
/// @param[out] pbuf Data buffer array or vector
/// @return Success/failure
static bool serialReadData(auto* pbuf)
{
    return serialReadData(pbuf, std::size(*pbuf));
}

/// @brief Read data into a buffer of given size
/// @param pbuf Data buffer
/// @param count Buffer size
/// @return Success/failure
static bool serialReadData(auto* pbuf, size_t count)
{
    std::span span(std::begin(*pbuf), count);
    for (auto&& ch : span) {
        int chIn = serialGetCharWait();
        if (chIn < 0) {
            Error::set<Error::Err::SerialIO>();
            //dputs("SerialIO: ERROR: timeout/error");
            return false;
        }
        ch = char(chIn);
        Watchdog::petTheDog();
    }
    return true;
}

/// @brief Empty out the serial input buffer by reading any available characters
void serialDrainInput()
{
    while (serialGetCharNoWait() >= 0) {
        tight_loop_contents();
    }
}

/// @brief Write data from a buffer of known size
/// @param buf Data buffer array or vector
/// @return Count of characters written, or a negative value on error
static int serialWriteData(const auto& buf)
{
    // The Pico SDK should have a better way to do this, e.g. _write_raw, like putchar_raw.
    // putchar_raw in a loop works correctly but is very slow.
    // Just calling _write by itself doesn't work because it inserts CR chars.
    // stdio_set_translate_crlf is not legally permitted after initialization,
    // but it's the only way available in Pico SDK 1.5.1 and it seems to work.
    // #kludge
    stdio_flush();
    bool translateCrlf = stdio_usb.crlf_enabled;
    stdio_set_translate_crlf(&stdio_usb, false);
    int count = _write(STDIO_HANDLE_STDOUT,
                    const_cast<char*>(&*std::begin(buf)), // that's C++ baby!
                    std::size(buf));
    stdio_flush();
    stdio_set_translate_crlf(&stdio_usb, translateCrlf);
    return count;
}

/// @brief Write a null-terminated string
/// @param str 
static void serialWriteLine(const char* str)
{
    puts(str);
}

/// @brief Write an acknowledgement after handling a command
static void serialWriteAck()
{
    serialWriteLine("OK");
}

} } // namespace SerialIO
