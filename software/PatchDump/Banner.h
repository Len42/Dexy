#pragma once
#include <iostream>
#include <format>
#include "system.h"
#include "version.h"

void PrintBanner()
{
    std::cout << std::format("{} {}, {}, C++{} {}\n",
        CommandLine::GetProgName(), Version::name,
        Version::compilerBuildConfig, Version::cppVersion,
        Version::compilerName);
}
