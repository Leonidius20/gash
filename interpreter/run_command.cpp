#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include "interpreter.h"

using namespace std;

namespace gash {

    int interpreter::visit(simple_command *node) {
        __pid_t new_proc_pid = fork();
        if (new_proc_pid < 0) {
            // failed to create child process
            printErrorForCommand("Failed to create child process", node->get_pathname(), true);
            return ERROR_VALUE;
        }
        if (new_proc_pid == 0) {
            // we are in the child process
            try_load_program(node->get_pathname());
        } else {
            // we are in parent process
            return monitor_child_process(new_proc_pid, node->get_pathname());
        }
    }

    void printErrorForCommand(const std::string& message, const std::string& pathname, bool useErrno) {
        cerr << "gash / simple_command runner: Error for simple_command " << pathname << ": "
            << message;
        if (useErrno) {
            cerr << " (" << strerror(errno) << ")";
            errno = 0;
        }
        cerr << '\n';
    }

}
