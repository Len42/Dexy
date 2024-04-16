// PatchDump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <format>
#include <vector>
#include <variant>
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

using PatchBank = std::variant<Dexy::Patches::V1::PatchBank, /*DEBUG*/int>;

// Patch file header fields
using cookie_t = uint32_t;
constexpr cookie_t serializeCookie = 'D'|('e'|('x'|('y'<<8))<<8)<<8; // little-endian
using version_t = uint16_t;
constexpr version_t versionMax = 2; // DEBUG 1;
constexpr size_t serializeHdrSize = sizeof(serializeCookie) + sizeof(version_t);

// Maximum file size, based on the largest version of serialized patch data
constexpr size_t maxFileSize = serializeHdrSize + Dexy::Patches::V1::patchBankSize;
// TODO: test with larger size

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
        // TODO: indexed access instead of switch - use "{std::in_place_index<i>}"
        switch (version) {
        case 1:
            patchBank = Dexy::Patches::V1::PatchBank(); // TODO: there must be a better way
            in(std::get<Dexy::Patches::V1::PatchBank>(patchBank)).or_throw();
            break;
        case 2:
            patchBank = int();
            in(std::get<int>(patchBank)).or_throw();
            break;
        default:
            throwError("Unrecognized patchbank version");
        }
        DPRINT("LoadPatchBank: Read patchbank version={}", version);
        return std::pair{ version, patchBank };
    } catch (std::system_error& ex) {
        // Improve the error messages from zpp::bits
        // TODO: don't format - just "Bad patch file contents" is fine
        std::string msg = std::format("Bad patch file contents ({}) -", ex.what());
        throwFileError(msg.c_str());
    }
}

static void DumpPatchBank(std::ostream& output,
                          const Dexy::Patches::V1::PatchBank& patchBank)
{
    DPRINT("DumpPatchBank: Dexy::Patches::V1::PatchBank");
    for (auto&& patch : patchBank.patches) {
        output << std::format("Patch: {}\n", "TODO"/*std::string_view(patch.name)*/);
    }
}

static void DumpPatchBank(std::ostream& output, int patchBank)
{
    DPRINT("DumpPatchBank: int");
    output << std::format("Bogus: {:x}\n", patchBank);
}

static void DumpPatchBank(std::ostream& output, const PatchBank& patchBank)
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

        std::ostream& output = std::cout;

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
        output << std::format("File: {}\n", inputFileName);

        auto storage = ReadFile(input);

        auto [version, patchBank] = LoadPatchBank(storage);

        output << std::format("Version: {}\n", version);

        DumpPatchBank(output, patchBank);

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << std::format("{}: ERROR: {}\n",
            CommandLine::GetProgName(), ex.what());
        return 1;
    }
}
