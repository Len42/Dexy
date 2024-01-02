// CritSec - Critical section helper class for Raspberry Pi Pico SDK
// by Len Popp

#pragma once

namespace Dexy {

/// @brief Critical section helper class that wraps critical_section_t
/// @tparam T Placeholder type that identifies a particular critical_section_t object
///
/// Usage
/// -----
/// @code
/// #include "CritSec.h"
///
/// struct CritSecForFunc { };
///
/// void init()
/// {
///     // Initialize a critical_section_t that is associated with any
///     // CritSec instance created with the same <CritSecForFunc> parameter
///     CritSec<CritSecForFunc>::init();
/// }
///
/// void func()
/// {
///     // ...
///     {
///         CritSec<CritSecForFunc> cs;
///         // Code in this block is protected by a critical_section_t
///     }
///     // ...
/// }
/// @endcode
///
/// Notes
/// -----
/// When compiling for the Raspberry Pi Pico, the C++ Standard Library does not
/// have support for thread concurrency (std::mutex etc.). Therefore the Pi Pico
/// SDK has its own concurrency support (spin_lock_t, critical_section_t, etc.).
/// This helper class simplifies the use of critical sections and reduces errors
/// by not requiring matching enter & exit calls. It generates efficient inlined
/// code which is identical to using critical_section_t explicitly (tested with
/// gcc 11.2).
///
/// To protect a block of code with a critical_section_t, declare a CritSec
/// object at the start of the scope block. The CritSec will block in its
/// constructor until the critical_section_t is available and it will release
/// the critical_section_t in its destructor at the end of the block.
///
/// All CritSec objects declared with the same template type, e.g. CritSec<AnyType>,
/// will share a single critical_section_t object. CritSec objects declared with
/// a different type, e.g. CritSec<SomeOtherType>, will use a different
/// critical_section_t object.
///
/// The block of code protected by a CritSec should be as short as possible,
/// typically just reading and setting a variable or two. Longer blocks of code
/// should use a mutex_t instead (which would have a different wrapper class;
/// not implemented).
template<typename T>
class CritSec
{
public:
    /// @brief Initialize a critical_section_t that is associated with all
    /// CritSec instances declared with the same T parameter
    static void init() { critical_section_init(&cs); }

    /// @brief De-initialize the critical_section_t
    /// @details This is meant to be called at shutdown, which means never.
    static void deinit() { critical_section_deinit(&cs); }

    /// @brief Ctor enters the critical section, blocking if necessary
    explicit CritSec() { critical_section_enter_blocking(&cs); }

    /// @brief Dtor leaves the critical section (at the end of the block where
    /// the object was declared)
    ~CritSec() { critical_section_exit(&cs); }

    // Move-only semantics
    CritSec(CritSec const&) = delete;
    CritSec& operator=(CritSec const&) = delete;
    CritSec(CritSec&&) noexcept = default;
    CritSec& operator=(CritSec&&) noexcept = default;

private:
    static inline critical_section_t cs;
};

}
