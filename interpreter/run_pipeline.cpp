#include <iostream>
#include <cstring>
#include <vector>

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h> // not all may be needed

#include "interpreter.h"

using namespace std;

namespace gash {

    int interpreter::visit(pipeline *node) {
        // build a pipeline and run programs here too?



        // I added this
        vector<__pid_t> child_proc_ids;
        int pipe_descriptors[2]; // [0] - read end, [1] - write end

        struct sigaction sigpipe_action{};
        sigpipe_action.sa_handler = SIG_IGN; // ignore signal
        sigpipe_action.sa_flags = 0;
        if (sigemptyset(&sigpipe_action.sa_mask) < 0) {
            throw runtime_error("pipeline runner: sigemptyset() failed");
        }

        /*
         * Функція pipeline_run() у циклі запускає кожну програму конвеєра.
            Стандартний потік виведення кожної програми конвеєра треба з’єднати
            зі стандартним потоком введення наступної програми конвеєра, тому на
            кожній ітерації циклу, крім останньої ітерації, створюється pipe, його
            дескриптори успадковуватимуться дочірнім процесом. При створенні pipe’а
            може бути помилка через досягнення максимальної кількості дескрипторів
            файлів у процесі або в системі, тому такі помилки не вважаються як
            системні помилки. Значення змінної read_fd дорівнює номеру дескриптора
            файлу кінця pipe’а для читання, який був створений у попередній ітерації
            циклу. Створивши новий процес, оригінальний процес має закрити
            дескриптори файлів відповідних кінців pipe’ів (read_fd та pipe_fd[1]), які були
            створені в попередній та поточній ітераціях циклу відповідно. Новий процес
            має закрити файловий дескриптор відповідного кінця pipe’а (pipe_fd[0]),
            який був створений у поточній ітерації циклу. Це дозволить лише одній
            програмі конвеєра бути власником дескриптора файлу будь-якого кінця
            будь-якого pipe’а, який створює функція pipeline_run(), що в свою чергу
            дозволить виявляти зламаний pipe.
         */

        /**
         * Дочірній процес 1) дублює дескриптор кінця pipe’а для читання, який був
            створений у попередній ітерації циклу (read_fd),
            у дескриптор файлу з номером STDIN_FILENO, якщо його номер не збігається з
            STDIN_FILENO та 2) дублює кінець pipe’а для запису, створений у поточній
            ітерації циклу (pipe_fd[1]), якщо його номер не збігається з STDOUT_FILENO.
            Системний виклик dup2() закриває дескриптор файлу, в який відбувається
            дублювання. Може бути так, що значення pipe_fd[1] збігається з STDIN_FILENO,
            тому дескриптор файлу з номером pipe_fd[1] треба дублювати в будь-який
            інший дескриптор файлу системним викликом dup() (до речі, дублювання
            може виконатися в дескриптор файлу з номером STDOUT_FILENO, якщо цей
            дескриптор файлу вільний). Всі ці дублювання дозволяють реалізувати
            зв’язок між стандартним потоком виведення одної програми конвеєра з
            стандартним потоком введення наступної програми конвеєра. Дочірній
            процес, виконавши всі необхідні дублювання, закриває оригінальні
            дескриптори файлів (ця дія просто закриває непотрібні дескриптори
            файлів).
         */

        /**
         * Дочірній процес ігнорує сигнал SIGPIPE. Це вказує ядру не надсилати цей
            сигнал процесу при спробі запису даних у зламаний pipe. Якщо програма
            конвеєра завершиться раніше ніж попередня програма конвеєра, яка
            виводить дані в стандартний потік виведення, тоді цей вивід даних буде
            виконуватись у зламаний pipe. Програма конвеєра може не очікувати
            отримати сигнал SIGPIPE, тому його треба ігнорувати до завантаження цієї
            програми (сигнал SIGPIPE можна блокувати, але програма може його
            розблокувати, при цьому не очікуючи його отримати).
            У випадку помилки в дочірньому процесі, яка виникає до завантаження
            нової програми або через неможливість завантажити нову програму,
            дочірній процес завершує своє виконання зі статусом нормального
            завершення процесу 127. Перевіряючи цей статус оригінальний процес
            може визначити зламаний конвеєр. Це не точна перевірка, оскільки
            завантажена програма також може завершити своє виконання зі статусом
            нормального завершення процесу 127. Можна скористатися функцією
            posix_spawnp(), але якщо ця функція не реалізована як системний виклик, тоді
            результат буде таким самим.

         */


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
                    // TODO: are throws good from child process? we get to main()
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

                // TODO: support group commands. Move the descriptor assigning code into
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