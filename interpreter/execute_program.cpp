#include <sys/wait.h>
#include <cstring>
#include <unistd.h>
#include "interpreter.h"

using namespace std;

namespace gash {

    /**
     * Load program into this process. This is meant to be executed in a child process.
     * If load fails, it kills the process and returns ERROR_VALUE as exit code.
     * @param pathname program's path
     */
    void try_load_program(const std::string& pathname) {
        // exec + l + p
        // l = program args passed in function args (only program path in this case. maybe it has to be only the file name instead of full path?)
        // p == PATH variable is considered
        if (execlp(pathname.c_str(), pathname.c_str(), nullptr) < 0) {
            // failed to load program
            printErrorForCommand("Failed to load program", pathname, true);
            // kill child process
            exit(ERROR_VALUE);
        }
    }

    /**
     * Wait for child process
     * @param pid pid of child process
     * @param pathname path of the program being executed in child process
     * @return its return code. If error, return 127
     */
    int monitor_child_process(int pid, const string& pathname) {
        int status = 0;

        int options = 0; // we don't want to return stopped & continued proc status,
        // wait for it to terminate instead
        if (waitpid(pid, &status, options) == -1) {
            printErrorForCommand("Error while waiting for process to terminate",
                                 pathname, true);
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

                printErrorForCommand(error_msg, pathname, false);

                return ERROR_VALUE + signal_number;
            } else {
                // what
                return ERROR_VALUE;
            }
        }

    }

}