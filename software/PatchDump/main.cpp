// PatchDump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <array>
#include <concepts>
#include <exception>

// Definitions for CmdLine.h
//#define CMDLINE_PROG_NAME "patchdump"
#define CMDLINE_PROG_DESCRIPTION "Dump a Dexy patchbank file in text format"
#define CMDLINE_ALLOW_ARGS true
#define CMDLINE_ARGS_DESCRIPTION "Dexy patchbank file (default stdin)"
#define CMDLINE_OPTIONS(ITEM) \
    ITEM(ShowVersion, v, ver, bool, false, "Display the software version at startup")
#include "CmdLine.h"
#include "Banner.h"

// header files from firmware
#include "Defs.h"
#include "Patches1.h"

// Patch file header fields
constexpr uint32_t serializeCookie = 'D'|('e'|('x'|('y'<<8))<<8)<<8; // little-endian
using version_t = uint16_t;
constexpr version_t versionMax = 1;
constexpr size_t serializeHdrSize = sizeof(serializeCookie) + sizeof(version_t);

static std::string inputFileName;

[[noreturn]] static void throwError(const char* message)
{
    throw std::runtime_error(message);
}

[[noreturn]] static void throwFileError(const char* message)
{
    throwError(std::format("{} {}", message, inputFileName).c_str());
}

template<std::integral NUM>
static bool ReadNum(std::istream& input, NUM* pnum)
{
    input.read(reinterpret_cast<char*>(pnum), sizeof(NUM));
    return bool(input);
}

static version_t ReadHeader(std::istream& input)
{
    uint32_t cookie;
    ReadNum(input, &cookie);
    if (!input || cookie != serializeCookie) {
        throwError("Bad patch file header");
    }
    version_t version;
    ReadNum(input, &version);
    if (!input || version == 0 || version > versionMax) {
        throwError("Bad patch file version");
    }
    return version;
}

static bool CheckEof(std::istream& input)
{
    if (input.eof()) {
        return false;
    } else {
        char ch;
        input >> ch;
        return input.eof();
    }
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
        // Throw an exception on stream error
        input.exceptions(std::istream::badbit);
        std::cout << std::format("Patch file: {}\n", inputFileName);

        version_t version = ReadHeader(input);
        
        // TODO: read appropriate type of patch

        // Check for EOF
        if (!CheckEof(input)) {
            throwFileError("Bad patch file - excess data");
        }

        // TODO: dump patch data

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << std::format("{}: ERROR: {}\n",
            CommandLine::GetProgName(), ex.what());
        return 1;
    }
}