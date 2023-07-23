#include <iostream>
#include <string>
#include <array>
#include <filesystem>
#include <utility>
#include <format>
#include "../../firmware/Defs.h"
#include "../../firmware/SynthAlgos.h"

using namespace Dexy;

const char* nameOf(Synth::UseMod mod)
{
    switch (mod) {
    case Synth::UseMod::none:
        return "UseMod.none";
    case Synth::UseMod::prev:
        return "UseMod.prev";
    case Synth::UseMod::saved:
        return "UseMod.saved";
    case Synth::UseMod::fb:
        return "UseMod.fb";
    default:
        return "???";
    }
}

const char* nameOf(Synth::SaveMod mod)
{
    switch (mod) {
    case Synth::SaveMod::none:
        return "SaveMod.none";
    case Synth::SaveMod::set:
        return "SaveMod.set";
    case Synth::SaveMod::add:
        return "SaveMod.add";
    default:
        return "???";
    }
}

int main(int argc, char* argv[])
{
    std::cout << "// DO NOT EDIT! This file was automatically generated to match the corresponding C++ definitions.\n";
    std::cout << "namespace Dexy.DexyPatch.Models\n{\n"
        "    public partial struct Algorithm\n    {\n";
    std::cout << "        /// <summary>\n";
    std::cout << "        /// Algorithm definitions\n";
    std::cout << "        /// </summary>\n";
    std::cout << std::format("        public static readonly Algorithm[] algorithms = new Algorithm[{}] {{\n", numAlgorithms);
    auto lineSepAlgorithm = "";
    for (auto&& algorithm : Synth::algorithms) {
        std::cout << std::format("{}            new Algorithm (new AlgoOp[{}] {{\n",
            std::exchange(lineSepAlgorithm, ",\n"), numOperators);
        auto lineSepAlgoOp = "";
        for (auto&& algoOp : algorithm.ops) {
            std::cout << std::format("{}                    new AlgoOp {{ ",
                std::exchange(lineSepAlgoOp, ",\n"));
            std::cout << std::format("isOutput={}, mod={}, saveMod={}, setsFb={} }}",
                algoOp.isOutput, nameOf(algoOp.mod), nameOf(algoOp.saveMod), algoOp.setFb);
        }
        std::cout << "\n                }\n";
        std::cout << "            )";
    }
    std::cout << "\n        };\n    }\n}\n";
}
