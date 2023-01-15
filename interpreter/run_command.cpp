#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <string.h>
#include "interpreter.h"

using namespace std;

namespace gash {

    // value returned if unable to launch program
    const int ERROR_VALUE = 127;

    int interpreter::visit(const unique_ptr<command> &node) {
        __pid_t new_proc_pid = fork();
        if (new_proc_pid < 0) {
            // failed to create child process
            printErrorForCommand("Failed to create child process", node->get_pathname(), true);
            return ERROR_VALUE;
        }
        if (new_proc_pid == 0) {
            // we are in the child process
            // exec + l + p
            // l = program args passed in function args (only program path in this case. maybe it has to be only the file name instead of full path?)
            // p == PATH variable is considered
            if (execlp(node->get_pathname().c_str(), node->get_pathname().c_str(), nullptr) < 0) {
                // failed to load program
                printErrorForCommand("Failed to load program", node->get_pathname(), true);
                // kill child process
                exit(ERROR_VALUE);
            }
        } else {
            // we are in parent process
            int status = 0;

            int options = 0; // we don't want to return stopped & continued proc status,
                             // wait for it to terminate instead
            if (waitpid(new_proc_pid, &status, options) == -1) {
                printErrorForCommand("Error while waiting for process to terminate",
                                     node->get_pathname(), true);
                return ERROR_VALUE;
            } else {
                // now we can use the 'status' variable to check what happened to the child process
                if (WIFEXITED(status)) {
                    // normal process exit
                    int exit_code = WEXITSTATUS(status); // only 8 bit
                    return exit_code;
                } else if (WIFSIGNALED(status)) {
                    // process ended by signal
                    int signal_number = WTERMSIG(status);
                    #ifdef WCOREDUMP
                    int was_core_dumped = WCOREDUMP(status);
                    #endif

                    string error_msg = string("Process ended by signal ") + strsignal(signal_number);
                    #ifdef WCOREDUMP
                    if (was_core_dumped) {
                        error_msg += ". Core was dumped";
                    }
                    #endif

                    printErrorForCommand(error_msg, node->get_pathname(), false);

                    return ERROR_VALUE + signal_number;
                }
            }
        }
        return 0;
    }

    void printErrorForCommand(const std::string& message, const std::string& pathname, bool useErrno) {
        cerr << "gash / command runner: Error for command " << pathname << ": "
            << message;
        if (useErrno) {
            cerr << " (" << strerror(errno) << ")";
            errno = 0;
        }
        cerr << '\n';
    }

}
