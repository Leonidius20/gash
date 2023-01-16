#include <iostream>
#include <cstring>
#include <vector>

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h> // not all may be needed

#include "interpreter.h"
#include "../ast/tree_node.h"

using namespace std;

namespace gash {

    int interpreter::visit(pipeline *node) {
        vector<__pid_t> child_proc_ids;
        int pipe_descriptors[2]; // [0] - read end, [1] - write end

        struct sigaction sigpipe_action{};
        sigpipe_action.sa_handler = SIG_IGN; // ignore signal
        sigpipe_action.sa_flags = 0;
        if (sigemptyset(&sigpipe_action.sa_mask) < 0) {
            throw runtime_error("pipeline runner: sigemptyset() failed");
        }

        int prev_read_fd; // descriptor of the read end of pipe from the previous iteration

        const vector<unique_ptr<command>> &commands = node->get_commands();
        for (auto cmd = commands.begin(); cmd != commands.end(); cmd++) {
            bool isFirst = cmd == commands.begin();
            bool isLast = cmd == commands.end() - 1;

            // create a new pipe if this is not the last program in the pipeline
            if (!isLast) {
                if (pipe(pipe_descriptors) < 0) {
                    // failed to create a pipe
                    if (!isFirst) {
                        close(prev_read_fd); // may fail but we are throwing an exception later anyway
                    }

                    if (errno == EMFILE || errno == ENFILE) {
                        // not enough descriptors to use
                        throw runtime_error(string("Error in pipeline runner: not enough descriptors (") + strerror(errno) + ")");
                    } else {
                        throw runtime_error(string("Error in pipeline runner: ") + strerror(errno) + ")");
                    }
                }
            }

            __pid_t child_pid = fork();

            if (child_pid < 0) {
                // error while forking. close descriptors
                if (!isFirst) {
                    close(prev_read_fd);
                }
                if (!isLast) {
                    close(pipe_descriptors[0]);
                    close(pipe_descriptors[1]);
                }

                throw runtime_error(string("Error in pipeline runner: ") + strerror(errno) + ")");
            }

            if (child_pid > 0) {
                // we are in parent process
                child_proc_ids.push_back(child_pid);

                if (!isFirst) {
                    // close prev read descriptor. At this point it has been already
                    // duplicated into the child process, so we don't need it here
                    if (close(prev_read_fd) < 0) {
                        printErrorForPipeline("when closing descriptor (1)", true);
                    }
                }

                if (!isLast) {
                    // close this iteration's write end descriptor. At this point it has been already
                    // duplicated into the child process, so we don't need it here
                    if (close(pipe_descriptors[1]) < 0) {
                        printErrorForPipeline("when closing descriptor (2)", true);
                    }
                }

                // saving this iteration's read end descriptor for the next iteration
                prev_read_fd = pipe_descriptors[0];
            } else {
                // we are in child process

                // ignoring SIGPIPE signals
                if (sigaction(SIGPIPE, &sigpipe_action, nullptr) < 0) {
                    // TODO: are throws good from child process?
                    throw runtime_error("pipeline runner: sigaction() failed");
                }

                if (!isLast) {
                    // we don't need the read end of this iteration's descriptor, as we will
                    // be reading from the previous iteration's read end
                    if (close(pipe_descriptors[0]) < 0) {
                        printErrorForPipeline("failed to close descriptor (3)", true);
                    }
                }

                if (!isFirst) {
                    // making the prev iteration's read end out STDIN, if it's not already

                    if (prev_read_fd != STDIN_FILENO) {
                        if (!isLast) {
                            // if this iteration's write end is assigned to STDIN for whatever reason
                            if (pipe_descriptors[1] == STDIN_FILENO) {
                                // create another descriptor for it instead
                                pipe_descriptors[1] = dup(pipe_descriptors[1]);
                                if (pipe_descriptors[1] < 0) {
                                    // that failed
                                    throw runtime_error("pipeline runner: error duplicating descriptor (1)");
                                }
                            }
                        }

                        // make prev iteration's read end this program's stdin
                        if (dup2(prev_read_fd, STDIN_FILENO) < 0) {
                            throw runtime_error("pipeline runner: error building pipe (1)");
                        }

                        // since the read end of pipe was assigned to STDIN, we don't need the
                        // other descriptor for it
                        if (close(prev_read_fd) < 0) {
                            printErrorForPipeline("error closing descriptor (4)", true);
                        }
                    }

                }

                if (!isLast) {
                    // making this iteration's write end our STDOUT, if it's not already
                    if (pipe_descriptors[1] != STDOUT_FILENO) {
                        // make this iteration's write end out STDOUT
                        if (dup2(pipe_descriptors[1], STDOUT_FILENO) < 0) {
                            throw runtime_error("pipeline runner: error building pipe (2)");
                        }

                        // since the write end of this iteration's pipe was assigned to STDOUT,
                        // we don't need this other descriptor for it anymore
                        if (close(pipe_descriptors[1]) < 0) {
                            printErrorForPipeline("error closing descriptor (5)", true);
                        }
                    }
                }

                // TODO: support group commands, remove cast. maybe Move the descriptor assigning code into
                // a method of its own, before try_load_program
                simple_command *s_cmd_ptr = static_cast<simple_command*>((*cmd).get());
                try_load_program(s_cmd_ptr->get_pathname());
            }
        }

        int return_code;
        for (__pid_t pid : child_proc_ids) {
            return_code = monitor_child_process(pid, "");
        }

        // at this point, return_code should contain the return code of the last program in pipe
        return return_code;
    }

    void printErrorForPipeline(const string& message, bool useErrno) {
        cerr << "gash / pipeline runner: "
             << message;
        if (useErrno) {
            cerr << " (" << strerror(errno) << ")";
            errno = 0;
        }
        cerr << '\n';
    }

}