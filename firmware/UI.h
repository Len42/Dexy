#pragma once

namespace Dexy {

/// @brief User interface using the display and rotary encoder
namespace UI {

/// @brief Task that implements the user interface, using a state machine
class UITask : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 50'000; }

    void init() override;

    void execute() override;

    /// @brief Notification that a note gate has started
    static void onGateStart();

    /// @brief Notification that a different patch has been selected
    /// (from outside of the UI module)
    static void onPatchSelected();

    /// @brief Notification that a new patch bank has been downloaded
    static void onPatchBankUpdate();
};

} } // namespace UI
