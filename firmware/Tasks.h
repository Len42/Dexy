#pragma once

namespace Dexy {
    
/// @brief A simple, efficient task scheduler for embedded microcontrollers
///
/// This task scheduler is designed to be simple to use and to have very little
/// overhead in time and memory. Some compile-time tricks are used to create a
/// static list of tasks, while keeping the application-level code simple with
/// a minimum of boilerplate.
/// 
/// Usage
/// -----
/// To define a set of tasks:
/// 
/// 1. Define each task as a subclass of Tasks::Task like this.
/// A single instance of this class will be created automatically.
/// @code
/// class ExampleTask : public Tasks::Task
/// {
/// public:
///     // Task execution interval in microseconds
///     unsigned intervalMicros() const override { return 1'000'000; }
/// 
///     // Task initialization, called once at program start
///     void init() override
///     {
///         // task initialization code
///     }
/// 
///     // Main task function, executed at (approximately) the specified interval
///     void execute() override
///     {
///         // task main code
///     }
/// };
/// @endcode
/// 2. Make a list of all the tasks by calling Tasks::makeTaskList.
/// Any tasks not currently required (e.g. for debugging) can be commented out
/// and the unused task code will not be compiled into the executable.
/// @code
/// constexpr auto taskList = Tasks::makeTaskList<
///     ExampleTask,
///     AnotherTask,
///     AndAnotherTask
/// >();
/// @endcode
/// 3. In main(), initialize all the tasks and then execute them repeatedly.
/// @code
/// int main()
/// {
///     // Initialize all the tasks.
///     taskList.initAll();
///
///     // Execute all the tasks repeatedly, at their specified time intervals.
///     while (true) {
///         taskList.runAll();
///     }
///
///     return 0;
/// }
/// @endcode
///
/// Acknowledgements
/// ----------------
/// Thanks to Luke Valenty for the slideware: https://youtu.be/fk0ihqOXER8
namespace Tasks {

/// @brief  Base class for application-defined tasks
class Task
{
public:
    /// @brief How often this task should be executed
    /// @return Task execution interval in microseconds
    virtual unsigned intervalMicros() const = 0;

    /// @brief Task initialization, called once at program start
    virtual void init() = 0;

    /// @brief Main task function, executed at approximately the specified interval
    virtual void execute() = 0;

    /// @brief If it's time to call execute(), do so
    /// @param now Current time
    void tick(absolute_time_t now)
    {
        if (timeIsReached(now, timer)) {
            timer = make_timeout_time_us(intervalMicros());
            execute();
        }
    }

private:
    /// @brief Keeps track of the next time this task should be executed
    absolute_time_t timer = from_us_since_boot_constexpr(0);
};

/// @brief There is one static instance of each subclass of Task
/// @tparam TASK_T A subclass of Task
template<typename TASK_T>
static TASK_T taskInstance;

/// @brief A static list of Task that is initialized at compile time
/// @tparam NUMTASKS The number of Task subclasses that have been declared
template<size_t NUMTASKS = 0>
class TaskList
{
public:
    consteval TaskList(const std::array<Task*, NUMTASKS>& _tasks) : tasks(_tasks) { }

    /// @brief Initialize all the tasks
    IN_FLASH("Tasks")
    void initAll() const
    {
        for (auto&& task : tasks) {
            task->init();
        }
    };

    /// @brief Execute all the tasks repeatedly, at their specified time intervals
    IN_FLASH("Tasks")
    void runAll() const
    {
        absolute_time_t now = get_absolute_time();
        for (auto&& task : tasks) {
            task->tick(now);
        }
    };

private:
    /// @brief List of Task instances to be executed
    const std::array<Task*, NUMTASKS> tasks;
};

/// @brief Add a task to a std::array of Task*
/// @tparam NUMTASKS 
/// @param taskList 
/// @param ptask 
/// @return 
template<size_t NUMTASKS>
consteval auto addTask(const std::array<Task*, NUMTASKS>& taskList, Task* ptask)
{
    std::array<Task*, NUMTASKS+1> newList;
    std::ranges::copy(taskList, std::begin(newList));
    newList[NUMTASKS] = ptask;
    return newList;
}

/// @brief Helper for makeTaskList()
/// @tparam NUMTASKS 
/// @param std::array of Task*
/// @return std::array of Task*
template<size_t NUMTASKS>
consteval auto makeTaskListHelper(const std::array<Task*, NUMTASKS>& taskList)
{
    return taskList;
}

/// @brief Helper for makeTaskList()
/// @tparam NUMTASKS 
/// @tparam TASK_T 
/// @tparam ...OTHER_TASKS 
/// @param taskList std::array of Task*
/// @return std::array of Task*
template<size_t NUMTASKS, class TASK_T, class... OTHER_TASKS>
consteval auto makeTaskListHelper(const std::array<Task*, NUMTASKS>& taskList)
{
    std::array<Task*, NUMTASKS+1> newList = addTask(taskList, &taskInstance<TASK_T>);
    return makeTaskListHelper<NUMTASKS+1, OTHER_TASKS...>(newList);
}

/// @brief Make a TaskList which is a compile-time constant containing the given
/// list of Task subclasses
/// @tparam ...TASKS A list of subclasses of Task
/// @return TaskList
template <class... TASKS>
consteval auto makeTaskList()
{
    auto taskList = makeTaskListHelper<0, TASKS...>(std::array<Task*, 0>{});
    return TaskList(taskList);
}

} } // namespace Tasks
