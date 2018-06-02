/**
 * @defgroup Policy
 *
 * @brief A policy describes how to execute a task.
 *
 * # Description
 *
 * When executing a task there are several ways to cope with failure.
 * An application might require to try to execute the task again, another one might need to stop when running them too many times.
 *
 * A policy specifies the rules of executing a task.
 *
 * # The Concept
 *
 * A class to qualify for the `Policy` concept needs to define a `execute()` method.
 * The method need to take a callable object (which implements the `Task` concept) and the arguments to invoke the callable with.
 * It needs to return a Failable.
 *
 */