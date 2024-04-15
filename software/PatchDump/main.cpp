// PatchDump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <format>
#include <array>

// header files from firmware
#include "Defs.h"
#include "Patches1.h"

// Definitions for CmdLine.h
//#define CMDLINE_PROG_NAME "patchdump"
#define CMDLINE_PROG_DESCRIPTION "Dump a Dexy patchbank file in text format"
#define CMDLINE_ALLOW_ARGS false
//#define CMDLINE_ARGS_DESCRIPTION "Input file"
#define CMDLINE_OPTIONS(ITEM) \
    ITEM(ShowVersion, v, ver, bool, false, "Display the software version at startup")
#include "CmdLine.h"
#include "Banner.h"

int main(int argc, char* argv[])
{
    try {
        if (CommandLine::Parse(argc, argv)) {
            return 0;
        }
        if (CommandLine::GetShowVersion()) {
            PrintBanner();
        }

        static constexpr char inputFileName[] = "stdin"; // TODO
        std::cout << std::format("Patch file: {}\n", inputFileName);

        // TODO: read patch file header
        // then read appropriate type of patch

        // TODO: dump patch data

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << std::format("{}: ERROR: {}\n",
            CommandLine::GetProgName(), ex.what());
        return 1;
    }
}