#pragma once

namespace Dexy {

/// @brief Class template for static tables of pre-calculated data
/// @tparam VALUE_T Type of table entries
/// @tparam NUM_VALUES Number of table entries
/// @tparam FUNC_CALC1 Function or lambda to calculate a single table entry
///
/// The data table is calculated and initialized at compile time.
/// The table contains NUM_VALUES values of type VALUE_T.
/// Each value is calculated by FUNC_CALC1 which has this signature:
/// @code
/// VALUE_T FUNC_CALC1(std::size_t index, std::size_t numValues)
/// @endcode
/// Acknowledgements
/// ----------------
/// Ashley Roll - https://github.com/AshleyRoll/cppcon21/blob/main/code/table_gen_1.cpp
///
/// Jason Turner - https://tinyurl.com/constexpr2021
template<typename VALUE_T, std::size_t NUM_VALUES, VALUE_T FUNC_CALC1(std::size_t index, std::size_t numValues)>
class DataTable
{
public:
    /// @brief Ctor initializes dataArray using FUNC_CALC1, at compile time
    consteval DataTable()
    {
        for (auto&& [index, value] : std::views::enumerate(dataArray)) {
            value = FUNC_CALC1(index, NUM_VALUES);
        }
    }

    /// @brief Size of the DataTable
    /// @return Size of the DataTable
    constexpr std::size_t size() const { return NUM_VALUES; }

    /// @brief The underlying array
    /// @return The underlying array
    auto& getArray() { return dataArray; }

    /// @brief The underlying array (const overload)
    /// @return The underlying array
    const auto& getArrayConst() const { return dataArray; }

    /// @brief Array subscript operator (non-const overload)
    /// @param index 
    /// @return Value at index
    constexpr VALUE_T& operator[](std::size_t index) { return dataArray[index]; }

    /// @brief Array subscript operator (const overload)
    /// @param index 
    /// @return Value at index
    constexpr VALUE_T operator[](std::size_t index) const { return dataArray[index]; }

private:
    /// @brief Fixed-size array of values that is initialized at compile time
    VALUE_T dataArray[NUM_VALUES];
};

}
