#pragma once

namespace Dexy {
    
/// @brief Serial USB input/output and command handling
namespace SerialIO {

/// @brief Task to handle serial USB I/O and execute commands
class SerialIOTask : public Tasks::Task
{
public:
    unsigned intervalMicros() const override { return 100'000; }

    void init() override;

    void execute() override;
};

} } // namespace SerialIO
