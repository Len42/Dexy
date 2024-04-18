// PatchDump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <variant>
#include <algorithm>
#include <concepts>
#include <exception>

// Definitions for CmdLine.h
//#define CMDLINE_PROG_NAME "patchdump"
#define CMDLINE_PROG_DESCRIPTION "Dump a Dexy patchbank file in text format"
#define CMDLINE_ALLOW_ARGS true
#define CMDLINE_ARGS_DESCRIPTION "Dexy patchbank file (default stdin)"
#define CMDLINE_OPTIONS(ITEM) \
    ITEM(ShowVersion, v, ver, bool, false, "Display the software version at startup") \
    ITEM(Debug, d, debug, bool, false, "Display debugging info")
#include "CmdLine.h"
#include "Banner.h"
#include "zpp_bits.h"

// header files from firmware
#include "Defs.h"
#include "Patches1.h"
#include "Patches2.h"

using PatchBank = std::variant<Dexy::Patches::V1::PatchBank,
                                Dexy::Patches::V2::PatchBank,
                                /*DEBUG*/int>;

// Patch file header fields
using cookie_t = uint32_t;
constexpr cookie_t serializeCookie = 'D'|('e'|('x'|('y'<<8))<<8)<<8; // little-endian
using version_t = uint16_t;
constexpr version_t versionMax = 3; // DEBUG 2;
constexpr size_t serializeHdrSize = sizeof(serializeCookie) + sizeof(version_t);

// Maximum file size, based on the largest version of serialized patch data
constexpr size_t maxFileSize = serializeHdrSize
    + std::max(Dexy::Patches::V1::patchBankSize, Dexy::Patches::V2::patchBankSize);

static std::string inputFileName;

// DPRINT - Print output only if the debug flag is set
#define DPRINT(msg, ...) \
    if (CommandLine::GetDebug()) { \
        std::cout << std::format(msg, __VA_ARGS__) << '\n'; \
    }

[[noreturn]] static void throwError(const char* message)
{
    throw std::runtime_error(message);
}

[[noreturn]] static void throwFileError(const char* message)
{
    throwError(std::format("{} {}", message, inputFileName).c_str());
}

static std::string_view TrimBlanks(std::string_view str)
{
    size_t start = str.find_first_not_of(" ");
    if (start == std::string_view::npos) {
        return std::string_view();
    } else {
        str.remove_prefix(start);
        size_t end = str.find_last_not_of(" ");
        if (end != std::string_view::npos) {
            str.remove_suffix(str.length() - end - 1);
        }
        return str;
    }
}

static auto ReadFile(std::istream& input)
{
    // Read one more byte than the max necessary, to check for eof.
    std::vector<char> storage(maxFileSize + 1);
    input.read(storage.data(), maxFileSize + 1);
    size_t numRead = input.gcount();
    DPRINT("ReadFile: numRead={}", numRead);
    if (numRead > maxFileSize) {
        throwFileError("Bad patch file length");
    }
    storage.resize(numRead);
    return storage;
}

static version_t CheckHeader(auto& in)
{
    cookie_t cookie;
    version_t version;
    in(cookie, version).or_throw();
    DPRINT("CheckHeader: cookie={:x} version={}", cookie, version);
    if (cookie != serializeCookie) {
        throwError("Bad patch file header");
    }
    if (version == 0 || version > versionMax) {
        throwError("Bad patch file version");
    }
    return version;
}

static auto LoadPatchBank(auto storage)
{
    try {
        PatchBank patchBank;
        zpp::bits::in in(storage);
        version_t version = CheckHeader(in);
        switch (version) {
        case 1:
            patchBank = Dexy::Patches::V1::PatchBank();
            in(std::get<Dexy::Patches::V1::PatchBank>(patchBank)).or_throw();
            break;
        case 2:
            patchBank = Dexy::Patches::V2::PatchBank();
            in(std::get<Dexy::Patches::V2::PatchBank>(patchBank)).or_throw();
            break;
        case 3:
            patchBank = int();
            in(std::get<int>(patchBank)).or_throw();
            break;
        default:
            throwFileError("Unrecognized patchbank version");
        }
        DPRINT("LoadPatchBank: Read patchbank version={}", version);
        // There should be no data left in the file
        if (!in.remaining_data().empty()) {
            throwFileError("Bad patch file length");
        }
        return std::pair{ version, patchBank };
    } catch (std::system_error& ex) {
        // Improve the error messages from zpp::bits
        // TODO: don't format - just "Bad patch file contents" is fine
        std::string msg = std::format("Bad patch file contents ({}) -", ex.what());
        throwFileError(msg.c_str());
    }
}

template<class PATCH, class OP>
static void DumpOpField(std::ostream& output, const PATCH& patch,
                        std::string_view name, auto OP::* pfield)
{
    output << name;
    for (auto&& op : patch.opParams) {
        output << std::format(",{}", op.*pfield);
    }
    output << '\n';
}

template<class PATCH, class OP, class SUB>
static void DumpOpField(std::ostream& output, const PATCH& patch,
                        std::string_view name, SUB OP::* psub, auto SUB::* pfield)
{
    output << name;
    for (auto&& op : patch.opParams) {
        output << std::format(",{}", op.*psub.*pfield);
    }
    output << '\n';
}

// TODO: Parameterize for V1 or V2:
// Make this function a template, call it DumpPatchCommon, call it from
// specialized overloads of DumpPatch
// TODO: maybe make a concept, just because?

template<class PATCH, class OP, class SUB>
static void DumpPatchCommon(std::ostream& output, const PATCH& patch)
{
    // TODO: oops, need template params for OpParams & EnvParams
    //////using namespace Dexy::Patches::V1;
    output << std::format("Patch,\"{}\"\n",
        TrimBlanks(std::string_view(std::begin(patch.name), std::end(patch.name))));
    output << std::format("Algorithm,{}\n", patch.algorithm + 1);
    output << std::format("Feedback,{}\n", patch.feedbackAmount);
    // Display per-operator fields in rows for readability
    DumpOpField(output, patch, "FixedFrequency", &OP::fixedFreq);
    // TODO: Smart display of MIDI note or frequency
    DumpOpField(output, patch, "NoteOrFrequency", &OP::noteOrFreq);
    DumpOpField(output, patch, "OutputLevel", &OP::outputLevel);
    DumpOpField(output, patch, "UseEnvelope", &OP::useEnvelope);
    DumpOpField(output, patch, "AmpModSens", &OP::ampModSens);
    DumpOpField(output, patch, "EnvDelay", &OP::env, &SUB::delay);
    DumpOpField(output, patch, "EnvAttack", &OP::env, &SUB::attack);
    DumpOpField(output, patch, "EnvDecay", &OP::env, &SUB::decay);
    DumpOpField(output, patch, "EnvSustain", &OP::env, &SUB::sustain);
    DumpOpField(output, patch, "EnvRelease", &OP::env, &SUB::release);
    DumpOpField(output, patch, "EnvLoop", &OP::env, &SUB::loop);
}

static void DumpPatch(std::ostream& output, const Dexy::Patches::V1::Patch& patch)
{
    DPRINT("DumpPatch V1");
    using namespace Dexy::Patches::V1;
    DumpPatchCommon<Patch, OpParams, EnvParams>(output, patch);
}

static void DumpPatch(std::ostream& output, const Dexy::Patches::V2::Patch& patch)
{
    DPRINT("DumpPatch V2");
    using namespace Dexy::Patches::V2;
    DumpPatchCommon<Patch, OpParams, EnvParams>(output, patch);
    output << "TODO: DumpPatch(V2): Other fields\n";
    DumpOpField(output, patch, "EnvRateScaling",
        &OpParams::env, &EnvParams::rateScaling);
    // TODO: Smart display of MIDI note or freq ratio
    DumpOpField(output, patch, "LScalBreak",
        &OpParams::levelScaling, &LevelScalingParams::breakPoint);
    // TODO: Smart display of curve type
    //DumpOpField(output, patch, "LScalCurveL",
    //    &OpParams::levelScaling, &LevelScalingParams::curveLeft);
    //DumpOpField(output, patch, "LScalCurveR",
    //    &OpParams::levelScaling, &LevelScalingParams::curveRight);
    {
        output << "LScalCurveL";
        for (auto&& op : patch.opParams) {
            output << std::format(",{}", int(op.levelScaling.curveLeft));
        }
        output << '\n';
        output << "LScalCurveR";
        for (auto&& op : patch.opParams) {
            output << std::format(",{}", int(op.levelScaling.curveRight));
        }
        output << '\n';
    }
    DumpOpField(output, patch, "LScalDepthL",
        &OpParams::levelScaling, &LevelScalingParams::depthLeft);
    DumpOpField(output, patch, "LScalDepthR",
        &OpParams::levelScaling, &LevelScalingParams::depthRight);
}

// TODO: maybe make a concept, just because?
template<class PATCHBANK>
static void DumpPatchBank(std::ostream& output, const PATCHBANK& patchBank)
{
    DPRINT("DumpPatchBank");
    for (auto&& patch : patchBank.patches) {
        DumpPatch(output, patch);
    }
}

static void DumpPatchBank(std::ostream& output, int patchBank)
{
    DPRINT("DumpPatchBank: int");
    output << std::format("Bogus,{:x}\n", patchBank);
}

static void DumpPatchBankVar(std::ostream& output, const PatchBank& patchBank)
{
    std::visit([&](auto&& obj) { DumpPatchBank(output, obj); }, patchBank);
}

int main(int argc, char* argv[])
{
    try {
        if (CommandLine::Parse(argc, argv)) {
            return 0;
        }
        if (CommandLine::GetShowVersion()) {
            PrintBanner();
        }

        // Open the input file
        bool inputFromStdin = false;
        std::ifstream inFile;
        auto args = CommandLine::GetOtherArgs();
        if (args.size() == 0) {
            inputFromStdin = true;
            inputFileName = "stdin";
        } else if (args.size() == 1) {
            inputFromStdin = false;
            inputFileName = args.front();
            inFile.open(inputFileName, std::ios::in | std::ios::binary);
            if (inFile.fail()) {
                throwFileError("Failed to open file");
            }
        } else {
            throwError("Too many file names");
        }
        std::istream& input = inputFromStdin ? std::cin : inFile;
        input.exceptions(std::istream::badbit); // Throw an exception on stream error

        std::ostream& output = std::cout;
        output << "Field,Value1,Value2,Value3,Value4,Value5,Value6\n";
        output << std::format("File,\"{}\"\n", inputFileName);

        auto storage = ReadFile(input);

        auto [version, patchBank] = LoadPatchBank(storage);

        output << std::format("Version,{}\n", version);

        DumpPatchBankVar(output, patchBank);

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << std::format("{}: ERROR: {}\n",
            CommandLine::GetProgName(), ex.what());
        return 1;
    }
}
