#pragma once

namespace Dexy { namespace Synth {

/// @brief The type of modulation used by an operator in an algorithm
/// @see AlgoOp Algorithm
enum class UseMod : char { none, prev, saved, fb };

/// @brief Whether an operator in an algorithm sets the saved modulation value
/// @see AlgoOp Algorithm
enum class SaveMod : char { none, set, add };

/// @brief The behaviour of a single operator in an algorithm
/// @see Algorithm
struct AlgoOp
{
    bool isOutput;    ///< If true, this operator is an output (carrier); if false, a modulator
    UseMod mod;       ///< Type of modulation that this operator _uses_
    SaveMod saveMod;  ///< Whether this operator _sets_ the saved modulation value
    bool setFb;       ///< If true, set the feedback modulation for later use
};

/// @brief Array of algorithm operator definitions
using AlgoOpArray = std::array<AlgoOp, numOperators>;

/// @brief An algorithm definition
/// @details Mainly a list of AlgoOps.
class Algorithm
{
private:
    /// @brief Helper for counting things in the AlgoOp array
    /// @param condition Boolean condition (function or lambda)
    /// @return Count of AlgoOps that satisfy the condition
    constexpr auto countIf(auto condition) const {
        return std::ranges::count_if(ops, condition);
    }

    /// @brief Count the number of outputs (carriers) in the AlgoOp array
    /// @details This gets called at compile time to initialize numOutputs,
    /// so use numOutputs instead of calling this.
    /// @return Count of output operators
    constexpr int countOutputs() const {
        return int(countIf([](auto&& op) {return op.isOutput;}));
    }

public:
    explicit constexpr Algorithm(const AlgoOpArray& opsIn)
        : ops(opsIn), numOutputs(countOutputs()) {}

    /// @brief The settings for all the operators, defining the algorithm
    AlgoOpArray ops;

    /// @brief How many of the operators are outputs (carriers)
    int numOutputs;

    /// @brief Is this algorithm definition valid?
    /// @return Yes or no
    constexpr bool isValid() const {
        // First operator cannot use modulation values that haven't been set yet
        if (ops[0].mod == UseMod::prev || ops[0].mod == UseMod::saved)
            return false;
        // Last operator must be an output not a modulator
        if (!ops[5].isOutput)
            return false;
        // Exactly one use of feedback
        if (countIf([](auto&& op){return op.mod == UseMod::fb;}) != 1)
            return false;
        // Exactly one source of feedback
        if (countIf([](auto&& op){return op.setFb;}) != 1)
            return false;
        // Outputs cannot set the saved modulation value
        if (countIf([](auto&& op) {return op.isOutput && op.saveMod != SaveMod::none; }) > 0)
            return false;
        bool fModSavedWasSet = false;
        for (auto&& op : ops) {
            // Saved modulation value must be initialized before it is used...
            if (op.mod == UseMod::saved && !fModSavedWasSet)
                return false;
            // ... or added to
            if (op.saveMod == SaveMod::add && !fModSavedWasSet)
                return false;
            if (op.saveMod == SaveMod::set)
                fModSavedWasSet = true;
        }
        return true;
    }

#ifdef DEBUG_DUMP_ALGORITHMS
    // dump - Print a diagram of the algorithm
    void dump() const {
        printf("%d outputs", numOutputs);
        bool fWasOutput = false;
        for (auto&& [i, op] : std::views::enumerate(ops)) {
            switch (op.mod) {
            case UseMod::none:
                if (!fWasOutput)
                    putchar('\n');
                break;
            case UseMod::prev:
                putchar('-');
                break;
            case UseMod::saved:
                putchar('=');
                break;
            case UseMod::fb:
                if (!fWasOutput)
                    putchar('\n');
                putchar('|');
                break;
            default:
                puts("ERROR");
                return;
            }
            printf("%d", 6-i); // DX7 convention: operators are numbered backwards
            if (op.setFb)
                putchar('|');
            if (op.saveMod == SaveMod::set)
                putchar('=');
            else if (op.saveMod == SaveMod::add)
                putchar('+');
            else
                putchar('-');
            if (op.isOutput)
                puts(">");
            fWasOutput = op.isOutput;
        }
        putchar('\n');
    }
#endif // DEBUG_DUMP_ALGORITHMS
};

/// @brief Table of Algorithm definitions - Same as DX7 algorithms
constexpr Algorithm algorithms[]
{
    //AlgoOp(isOutput, useMod, saveMod, setFb),
    Algorithm( { // 1
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 2
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 3
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 4
    AlgoOp(false, UseMod::fb, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, true),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 5
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 6
    AlgoOp(false, UseMod::fb, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, true),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 7
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 8
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::fb, SaveMod::add, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 9
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 10
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 11
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 12
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 13
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 14
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(false, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 15
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(false, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 16
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::add, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    } ),
    Algorithm( { // 17
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::add, false),
    AlgoOp(false, UseMod::fb, SaveMod::add, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    } ),
    Algorithm( { // 18
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::set, false),
    AlgoOp(false, UseMod::fb, SaveMod::add, true),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    } ),
    Algorithm( { // 19
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 20
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    } ),
    Algorithm( { // 21
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    } ),
    Algorithm( { // 22
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 23
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 24
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 25
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 26
    AlgoOp(false, UseMod::fb, SaveMod::set, true),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 27
    AlgoOp(false, UseMod::none, SaveMod::set, false),
    AlgoOp(false, UseMod::none, SaveMod::add, false),
    AlgoOp(true, UseMod::saved, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 28
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    } ),
    Algorithm( { // 29
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(false, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 30
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(false, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 31
    AlgoOp(false, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::prev, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } ),
    Algorithm( { // 32
    AlgoOp(true, UseMod::fb, SaveMod::none, true),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    AlgoOp(true, UseMod::none, SaveMod::none, false),
    } )
};

} } // namespace Synth
