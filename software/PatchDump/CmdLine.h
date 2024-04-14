#pragma once

#include <string>
#include <string_view>
#include <charconv>
#include <vector>
#include <span>
#include <ranges>
#include <concepts>
#include <type_traits>
#include <filesystem>

#ifndef CMDLINE_OPTIONS
#error CMDLINE_OPTIONS(item) must be defined before #including CmdLine.h
#endif
#ifndef CMDLINE_PROG_NAME
#define CMDLINE_PROG_NAME ""
#endif
#ifndef CMDLINE_PROG_DESCRIPTION
#define CMDLINE_PROG_DESCRIPTION ""
#endif
#ifndef CMDLINE_ALLOW_ARGS
#define CMDLINE_ALLOW_ARGS true
#endif
#ifndef CMDLINE_ARGS_DESCRIPTION
#define CMDLINE_ARGS_DESCRIPTION ""
#endif

using namespace std::literals;

/// <summary>
/// A class to parse a program's command line into a set of defined options
/// and arguments
/// </summary>
/// <remarks>
/// The functions in this class are static. Don't create an instance of this
/// class, just call the functions.
/// A "--help" option is always implemented; it doesn't need to be defined in
/// CMDLINE_OPTIONS.
/// </remarks>
/// <example>
/// Examples of command lines:
/// <code>
/// xyzzy --verbose --max-lines=1000 file
/// xyzzy -vr -m=1000 file1 file2 file3
/// xyzzy --no-reverse file
/// </code>
/// </example>
/// <example>
/// This is how to define a program's command-line options.
/// CMDLINE_OPTIONS must be defined before #including CmdLine.h.
/// The other macros are optional.
/// <code>
/// #define CMDLINE_PROG_NAME "xyzzy"
/// #define CMDLINE_PROG_DESCRIPTION "Do something interesting."
/// #define CMDLINE_ALLOW_ARGS true
/// #define CMDLINE_ARGS_DESCRIPTION "Input filenames"
/// #define CMDLINE_OPTIONS(ITEM) \
///     /* ITEM(id, nameShort, nameLong, valType, defVal, help) */ \
///     ITEM(Verbose, v, verbose, bool, false, "Output extra diagnostic info")
///     ITEM(Reverse, r, reverse, bool, false, "Do everything backwards")
///     ITEM(MaxLines, m, max-lines, size_t, 100, "Maximum number of lines to output") \
/// #include "CmdLine.h"
/// </code>
/// In the program's main(), CommandLine::Parse() sets the argument values from
/// the command line, after which their values are available.
/// <code>
/// int main(int argc, char* argv[])
/// {
///     try {
///         if (CommandLine::Parse(argc, argv)) {
///             // The program was run with "--help" so just exit after the
///             // help message has been displayed.
///             return 0;
///         }
/// 
///         // GetVerbose() gets the value of the option with identifier Verbose.
///         if (CommandLine::GetVerbose()) {
///             std::cout << "output some extra stuff";
///         }
/// 
///         // GetOtherArgs() returns all the unnamed (non-option) arguments
///         // on the command line.
///         for (auto&& fileName : CommandLine::GetOtherArgs()) {
///             DoSomethingWithFile(fileName);
///         }
/// 
///         return 0;
///     } catch (const std::exception& ex) {
///         // GetProgName() is used to put the program name in error messages.
///         std::cerr << std::format("{}: ERROR: {}\n",
///             CommandLine::GetProgName(), ex.what());
///         return 1;
///     }
/// }
/// </code>
/// </example>
class CommandLine
{
// Interface
public:
    /// <summary>
    /// Process the options and arguments on the command line and store their
    /// values.
    /// </summary>
    /// <remarks>
    /// This should be called at the beginning of main().
    /// </remarks>
    /// <param name="argc">argc as passed to main()</param>
    /// <param name="argv">argv as passed to main()</param>
    /// <returns>true if the --help message was displayed, false otherwise</returns>
    /// <exception cref="std::exception"></exception>
    static bool Parse(int argc, char* argv[])
    {
        auto args = std::span<char*>(argv, argc);
        if (progName.empty()) {
            if (!args.empty()) {
                progName = std::filesystem::path(args[0]).stem().string();
            }
            if (progName.empty()) {
                progName = "<progname>";
            }
        }
        auto argList = args | std::views::drop(1);
        for (auto iter = argList.begin(); iter != argList.end(); ++iter) {
            auto arg = std::string_view(*iter);
            if (arg.empty()) {
                throwCmdLineError("Empty command line argument");
            } else if (arg.starts_with("--"sv) && arg.size() > 2) {
                HandleLongName(arg.substr(2));
            } else if (arg.starts_with("-"sv) && arg.size() > 1) {
                HandleShortName(arg.substr(1));
            } else {
                if (allowOtherArgs) {
                    otherArgs.push_back(std::string(arg));
                } else {
                    throwCmdLineError("Unnamed command line arguments not allowed");
                }
            }
        }
        if (namedOptions.showHelp) {
            PrintHelpMessage(std::cout);
            return true;
        }
        return false;
    }

    /// <summary>
    /// Get the program name, as shown in the "usage" message and in error
    /// messages.
    /// </summary>
    /// <remarks>
    /// By default this is the executable filename excluding path and extension.
    /// It can be customized by CMDLINE_PROG_NAME.
    /// </remarks>
    /// <returns>The program name</returns>
    static constexpr std::string_view GetProgName() { return progName; }

    /// <summary>
    /// Get a list of the non-option command-line arguments.
    /// </summary>
    /// <returns>A view on the list of argument strings</returns>
    static constexpr std::ranges::view auto GetOtherArgs()
        { return std::views::all(otherArgs); }

    // Getter functions for the program-specific command line options
#define DECLARE_GET_OPTION(id, nameShort, nameLong, valType, ...) \
    static const auto& Get##id() { return namedOptions.option##id; }
    CMDLINE_OPTIONS(DECLARE_GET_OPTION)

// Data
private:
#define DECLARE_OPTION_ID(id, ...) id,
    enum class OptionId : unsigned {
        CMDLINE_OPTIONS(DECLARE_OPTION_ID) Help
    };

#define COUNT_OPTIONS(...) +1
    static constexpr unsigned numOptions =
        CMDLINE_OPTIONS(COUNT_OPTIONS) + 1/* for --help */;

    struct OptionInfo
    {
        OptionId id;
        bool hasValue;
        std::string_view nameShort;
        std::string_view nameLong;
        std::string_view description;
        std::string_view defValue;
        // Note defValue is a string representation of the default value, used
        // for documentation; the actual value is used elsewhere for initialization.
    };
#define DECLARE_OPTION_INFO(id, nameShort, nameLong, valType, defVal, help, ...) \
    { OptionId::id, !std::is_same<valType, bool>::value, #nameShort##sv, #nameLong##sv, help##sv, #defVal##sv },
    static constexpr OptionInfo optionInfo[numOptions] = {
        CMDLINE_OPTIONS(DECLARE_OPTION_INFO)
        { OptionId::Help, false, "h"sv, "help"sv, "Display this message"sv, "false"sv }
    };

    static inline std::string progName = CMDLINE_PROG_NAME;

    static inline std::string progDescription = CMDLINE_PROG_DESCRIPTION;

#define DECLARE_OPTION(id, nameShort, nameLong, valType, defVal, ...) \
    valType option##id;
    struct CmdOptions
    {
        CMDLINE_OPTIONS(DECLARE_OPTION)
        bool showHelp;
    };
#define DEFAULT_OPTION(id, nameShort, nameLong, valType, defVal, ...) defVal,
    static inline CmdOptions namedOptions {
        CMDLINE_OPTIONS(DEFAULT_OPTION) false/*showHelp*/
    };

    static inline bool allowOtherArgs = CMDLINE_ALLOW_ARGS;

    static inline std::vector<std::string> otherArgs;

    static inline std::string argsDescription = CMDLINE_ARGS_DESCRIPTION;

// Implementation
private:
    static const OptionInfo& GetOptionInfo(OptionId id)
    {
        //assert(unsigned(id) < std::size(optionInfo));
        //assert(optionInfo[unsigned(id)].id == id);
        return optionInfo[unsigned(id)];
    }

    static void HandleShortName(std::string_view option)
    {
        auto [fYes, option2] = CheckYesNo(option);
        auto [value, option3] = CheckValue(option2);
        // Handle several single-char options packed together, but
        // not if "no-" or "=" is given (for simplicity).
        if (!fYes || !value.empty()) {
            // Treat as a single option with a value specified
            OptionId optionId = FindOptionShort(option3);
            HandleOption(optionId, fYes, value);
        } else {
            // Treat option as a set of single-char flags, e.g. "ls -al"
            for (size_t i = 0; i < option3.size(); ++i) {
                std::string_view option = option3.substr(i, 1);
                OptionId optionId = FindOptionShort(option);
                HandleOption(optionId, fYes, value);
            }
        }
    }

    static void HandleLongName(std::string_view option)
    {
        auto [fYes, option2] = CheckYesNo(option);
        auto [value, option3] = CheckValue(option2);
        OptionId optionId = FindOptionLong(option3);
        HandleOption(optionId, fYes, value);
    }

    // returns yes/no, option
    static std::pair<bool, std::string_view> CheckYesNo(std::string_view option)
    {
        bool fYes = true;
        if (option.starts_with("no-")) {
            fYes = false;
            option = option.substr(3);
        }
        return { fYes, option };
    }

    // returns value, option
    static std::pair<std::string_view, std::string_view> CheckValue(std::string_view option)
    {
        size_t pos = option.find('=');
        if (pos == std::string_view::npos) {
            return { std::string_view(), option };
        } else {
            return { option.substr(pos + 1), option.substr(0, pos) };
        }
    }

    static OptionId FindOption(std::string_view option, std::predicate<OptionInfo> auto predicate)
    {
        auto found = std::ranges::find_if(optionInfo, predicate);
        if (found != std::end(optionInfo)) {
            return found->id;
        } else {
            std::string msg = std::format("Unrecognized command line option '{}'", option);
            throwCmdLineError(msg.c_str());
        }
    }

    static OptionId FindOptionShort(std::string_view option)
    {
        return FindOption(option, [&](auto&& info) { return option == info.nameShort; });
    }

    static OptionId FindOptionLong(std::string_view option)
    {
        return FindOption(option, [&](auto&& info) { return option == info.nameLong; });
    }

    static void HandleOption(OptionId id, bool fYes, std::string_view value)
    {
        if (GetOptionInfo(id).hasValue) {
            SetOptionValue(id, value);
        } else {
            SetFlagValue(id, fYes, value);
        }
    }

    template<typename NUM> requires std::integral<NUM>
    static std::optional<NUM> ToNumber(std::string_view str)
    {
        NUM value;
        const char* ptrEnd = str.data() + str.size();
        auto [ptr, ec] = std::from_chars(str.data(), ptrEnd, value);
        if (ec == std::errc() && ptr == ptrEnd) {
            return value;
        } else {
            return std::nullopt;
        }
    }

    static std::optional<bool> ToBool(std::string_view str)
    {
        std::optional<int> num = ToNumber<int>(str);
        if (num) {
            if (*num == 0 || *num == 1) {
                return bool(*num);
            } else {
                return std::nullopt;
            }
        } else {
            if (str == "yes"sv || str == "true"sv) {
                return true;
            } else if (str == "no"sv || str == "false"sv) {
                return false;
            } else {
                return std::nullopt;
            }
        }
    }

    static void SetFlagValue(OptionId id, bool fYes, std::string_view value)
    {
        if (!value.empty()) {
            bool ok = fYes; // can't have both "no-..." and "...=true"
            if (ok) {
                // Get fYes from value
                auto boolValue = ToBool(value);
                if (boolValue) {
                    fYes = *boolValue;
                } else {
                    ok = false;
                }
            }
            if (!ok) {
                const OptionInfo& info = GetOptionInfo(id);
                std::string msg = std::format("Invalid boolean value for command line option '-{}'/'--{}'", info.nameShort, info.nameLong);
                throwCmdLineError(msg.c_str());
            }
        }
#define SET_FLAG_OPTION(id, nameShort, nameLong, ...) case id: namedOptions.option##id = fYes; break;
        switch (id) {
            using enum OptionId;
        case Help: namedOptions.showHelp = fYes; break;
            CMDLINE_OPTIONS(SET_FLAG_OPTION)
        }
    }

    static bool SetOptionValue(std::string& member, std::string_view value) {
        member = value;
        return true;
    }

    template<typename NUM> requires std::integral<NUM>
    static bool SetOptionValue(NUM& valueDest, std::string_view valueStr)
    {
        auto result = ToNumber<NUM>(valueStr);
        if (result) {
            valueDest = *result;
            return true;
        } else {
            return false;
        }
    }

    static bool SetOptionValue(bool& value, std::string_view option)
    {
        // shouldn't happen!
        throwInternalError();
    }

    static void SetOptionValue(OptionId id, std::string_view option)
    {
        bool ok = false;
#define SET_OPTION_VALUE(id, ...) case id: ok = SetOptionValue(namedOptions.option##id, option); break;
        switch (id) {
            using enum OptionId;
            CMDLINE_OPTIONS(SET_OPTION_VALUE)
        // dummy entry for Help - not needed but the compiler wants it
        case Help: throwInternalError();
        }
        if (!ok) {
            const OptionInfo& info = GetOptionInfo(id);
            std::string msg = std::format("Invalid value for command line option '-{}'/'--{}'", info.nameShort, info.nameLong);
            throwCmdLineError(msg.c_str());
        }
    }

    static void PrintUsageMessage(std::ostream& str)
    {
        str << "Usage: " << progName;
        for (auto&& info : optionInfo) {
            str << std::format(" [-{}|--{}{}]",
                info.nameShort, info.nameLong,
                (info.hasValue ? "=<value>" : ""));
        }
        if (allowOtherArgs) {
            str << " <args...>";
        }
        str << '\n';
    }

    static std::string GetUsageMessage()
    {
        std::ostringstream str;
        PrintUsageMessage(str);
        return str.str();
    }

    static void PrintHelpMessage(std::ostream& str)
    {
        PrintUsageMessage(str);
        str << '\n';
        if (!progDescription.empty()) {
            str << progDescription << "\n\n";
        }
        for (auto&& info : optionInfo) {
            std::string nameLong;
            if (info.hasValue) {
                nameLong = std::format("{}=<value>", info.nameLong);
            } else {
                nameLong = info.nameLong;
            }
            str << std::format("    -{}, --{:16}{} (default {})\n",
                info.nameShort, nameLong, info.description, info.defValue);
        }
        if (allowOtherArgs) {
            if (argsDescription.empty()) {
                argsDescription = "Command arguments";
            }
            str << std::format("    <args...>             {}\n", argsDescription);
        }
    }

    class error_t : public std::exception
    {
    public:
        error_t() = delete;
        explicit error_t(const char* message) : std::exception(message) {}
    };

    [[noreturn]]
    static void throwError(const char* message)
    {
        throw error_t(message);
    }

    [[noreturn]]
    static void throwInternalError()
    {
        throwError("Internal error in command line processing");
    }

    [[noreturn]]
    static void throwCmdLineError(const char* errorMessage)
    {
        std::string usage = GetUsageMessage();
        std::string message = std::format("{}\n{}", errorMessage, usage);
        throwError(message.c_str());
    }
};
