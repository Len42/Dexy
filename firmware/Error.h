#pragma once

namespace Dexy {
    
/// @brief Error reporting
namespace Error {

/// @brief List of errors
/// @see https://en.wikipedia.org/wiki/X_macro
#define FOR_EACH_ERROR_TYPE(DO) \
    DO(DataNotReady) \
    DO(WrongIrqGpio) \
    DO(WrongIrqEvent) \
    DO(WrongCore) \
    DO(Watchdog) \
    DO(BadPatchData) \
    DO(SerialIO) \
    DO(Lockout) \
    DO(BadFlashData) \
    DO(BadArgument) \
    DO(Whatever)

/// @brief Error codes
enum class Err : unsigned {
#define DECLARE_ERR_VALUE(name, ...) name,
    FOR_EACH_ERROR_TYPE(DECLARE_ERR_VALUE)
};

/// @brief Set an error code
/// @tparam err Error code
template<Err err>
void set();

/// @brief Is a particular error code set?
/// @tparam err Error code
/// @return Is it set?
template<Err err>
bool isSet();

/// @brief Clear an error code
/// @tparam err Error code
template<Err err>
void clear();

/// @brief Set an error if an assertion fails
/// @tparam err Error code
/// @param f Boolean expression that should always be true
template<Err err>
void assertion(bool f);

/// @brief Is any error code set?
/// @return Yes or no
bool anySet();

/// @brief Clear all errors
void clearAll();

/// @brief Print a list of all the errors that are currently set, to the debug
/// output stream.
void dumpAllSetErrors();

} // namespace Error

#ifdef DEBUG_CHECK_ERRORS
/// @brief Set an error if an assertion fails
/// @param cond Boolean expression that should always be true
/// @param err Error code
#define dassert(cond, err) Error::assertion<Error::Err::err>(cond)
#else
#define dassert(cond, err)
#endif

}
