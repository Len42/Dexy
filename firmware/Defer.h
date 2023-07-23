#pragma once

namespace Dexy {
    
/// @brief Asynchronous function execution, oversimplified
/// @details
/// This is a simple way to implement asynchronous function calls that are
/// executed by a different core or Dexy::Tasks::Task. It only supports simple
/// functions with no return value. A mutex may be used to ensure synchronization
/// between cores. (The mutex is optional because it's only needed when calling
/// between cores, not between tasks.)
///
/// Usage
/// -----
/// A function that is called _without_ using a mutex must be declared like this:
/// @code
///     void myFunction(unsigned arg) { ... }
/// @endcode
/// To force a mutex to be used for cross-core safety, declare the function with
/// a dummy UseCritSec argument: 
/// @code
///     void myFunction(Defer::UseCritSec, unsigned arg) { ... }
/// @endcode
/// The core or Task that implements the function must periodically call
/// checkRun() to execute pending function calls:
/// @code
///     Defer::checkRun<myFunction>();
/// @endcode
/// To call the function from a different core or Task it must be called via
/// Defer::call():
/// @code
///     Defer::call<myFunction>(arg);
/// @endcode
///
/// Notes
/// -----
/// Only a single mutex (actually a critical_section_t) is used for all deferred
/// functions. It might make sense to use a separate mutex for every function but
/// that would be a huge pain. The single mutex is not currently a bottleneck so
/// let's keep it simple.
namespace Defer {

/// @brief Initialization - must be called at startup
void init();

// Deferred calls without mutex

using FuncTypeNoMutex = void (*)(unsigned);

/// @brief Make a deferred call to a function (no-mutex overload)
/// @tparam func 
/// @param arg 
template<FuncTypeNoMutex func>
void call(unsigned arg);

/// @brief Make a deferred call to a function (no-mutex overload)
/// @tparam func 
/// @details This overload may be used for a function that ignores its argument.
template<FuncTypeNoMutex func>
void call();

/// @brief Check if there is a function call pending and if so execute the function
/// (no-mutex overload)
/// @tparam func 
/// @return true if the function was executed, false if not
template<FuncTypeNoMutex func>
bool checkRun();

/// @brief Cancel the pending deferred call to a function (no-mutex overload)
/// @tparam func 
template<FuncTypeNoMutex func>
void clearPending();

// Deferred calls with mutex

struct UseCritSec { };

using FuncTypeMutex = void (*)(UseCritSec, unsigned);

/// @brief Make a deferred call to a function (with-mutex overload)
/// @tparam func 
/// @param arg 
template<FuncTypeMutex func>
void call(unsigned arg);

/// @brief Make a deferred call to a function (with-mutex overload)
/// @tparam func 
/// @details This overload may be used for a function that ignores its argument.
template<FuncTypeMutex func>
void call();

/// @brief Check if there is a function call pending and if so execute the function
/// (with-mutex overload)
/// @tparam func 
/// @return true if the function was executed, false if not
template<FuncTypeMutex func>
bool checkRun();

/// @brief Cancel the pending deferred call to a function (with-mutex overload)
/// @tparam func 
template<FuncTypeMutex func>
void clearPending();

// Implementation

/// @brief Mutex (actually CritSec) to synchronize caller and implemeter
using CritSecDefer = CritSec<UseCritSec>;

void init()
{
    CritSecDefer::init();
}

/// @brief Call-pending flag for a particular function
/// @tparam FuncType FuncTypeMutex or FuncTypeNoMutex
/// @tparam func 
template<typename FuncType, FuncType func>
static bool pending = false;

static constexpr unsigned invalidArg = std::numeric_limits<unsigned>::max();

/// @brief Argument for a pending call to a particular function
/// @tparam FuncType FuncTypeMutex or FuncTypeNoMutex
/// @tparam func 
template<typename FuncType, FuncType func>
static unsigned funcArg = invalidArg;

template<FuncTypeNoMutex func>
void call(unsigned arg)
{
    funcArg<FuncTypeNoMutex, func> = arg;
    pending<FuncTypeNoMutex, func> = true;
}

template<FuncTypeNoMutex func>
void call()
{
    call<func>(invalidArg);
}

template<FuncTypeNoMutex func>
bool checkRun()
{
    if (std::exchange(pending<FuncTypeNoMutex, func>, false)) {
        func(funcArg<FuncTypeNoMutex, func>);
        return true;
    } else {
        return false;
    }
}

template<FuncTypeNoMutex func>
void clearPending()
{
    pending<FuncTypeNoMutex, func> = false;
}

template<FuncTypeMutex func>
void call(unsigned arg)
{
    CritSecDefer cs;
    funcArg<FuncTypeMutex, func> = arg;
    pending<FuncTypeMutex, func> = true;
}

template<FuncTypeMutex func>
void call()
{
    call<func>(invalidArg);
}

template<FuncTypeMutex func>
bool checkRun()
{
    bool doIt;
    unsigned arg;
    {
        CritSecDefer cs;
        doIt = std::exchange(pending<FuncTypeMutex, func>, false);
        arg = funcArg<FuncTypeMutex, func>;
    }
    if (doIt) {
        func(UseCritSec(), arg);
    }
    return doIt;
}

template<FuncTypeMutex func>
void clearPending()
{
    CritSecDefer cs;
    pending<FuncTypeMutex, func> = false;
}

} } // namespace Defer
