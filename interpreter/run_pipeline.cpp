#include <iostream>
#include <cstring>

#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h> // not all may be needed

#include "interpreter.h"

using namespace std;

namespace gash {

    struct prog_descr {
        const char *name;
        char **argv;
        pid_t pid;
    };

    int interpreter::visit(const unique_ptr<pipeline> &node) {
        // build a pipeline and run programs here too?

        struct sigaction act;
        int read_fd, pipe_fd[2];
        bool first, last;
        act.sa_handler = SIG_IGN;
        act.sa_flags = 0;
        if (sigemptyset(&act.sa_mask) < 0)
            exit_err("pipeline_run: sigemptyset()");

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
        for (first = true; descr->name != NULL; first = false, ++descr) {
            last = (descr + 1)->name == NULL; // is this the last program in pipe?

            // if unable to create pipe
            if (!last && pipe(pipe_fd) < 0) {
                if (errno != EMFILE && errno != ENFILE)
                    exit_err("pipeline_run: pipe()");
                warn_err("pipeline_run: pipe()");
                if (!first && close(read_fd) < 0) // if not first in pipe, close the read end
                    exit_err("pipeline_run: close()");
                descr->pid = -1;
                break;
            }
            descr->pid = fork(); // create child process

            // if error while forking, close everything
            if (descr->pid < 0) {
                warn_err("pipeline_run: fork()");
                if ((!first && close(read_fd) < 0) ||
                    (!last && (close(pipe_fd[0]) < 0 || close(pipe_fd[1]) < 0))
                        ) {
                    exit_err("pipeline_run: close()");
                }
                break;
            }

            // if we are in the child process
            if (descr->pid > 0) {
                if ((!first && close(read_fd) < 0) || // close extra descriptors
                    (!last && close(pipe_fd[1]) < 0)
                        ) {
                    exit_err("pipeline_run: close()");
                }
                read_fd = pipe_fd[0];
            } else { // if we are parent process
                if (sigaction(SIGPIPE, &act, NULL) < 0)      // set to ignore SIGPIPE singnals?
                    _exit_err("pipeline_run: sigaction()");
                if (!last && close(pipe_fd[0]) < 0)
                    _exit_err("pipeline_run: close()");
                if (!first && read_fd != STDIN_FILENO) {
                    if (!last && pipe_fd[1] == STDIN_FILENO) {
                        pipe_fd[1] = dup(pipe_fd[1]);
                        if (pipe_fd[1] < 0)
                            _exit_err("pipeline_run: dup()");
                    }
                    if (dup2(read_fd, STDIN_FILENO) < 0)
                        _exit_err("pipeline_run: dup2()");
                    if (close(read_fd) < 0)
                        _exit_err("pipeline_run: close()");
                }
                if (!last && pipe_fd[1] != STDOUT_FILENO) {
                    if (dup2(pipe_fd[1], STDOUT_FILENO) < 0)
                        _exit_err("pipeline_run: dup2()");
                    if (close(pipe_fd[1]) < 0)
                        _exit_err("pipeline_run: close()");
                }
                execvp(descr->name, descr->argv);
                _exit_err("pipeline_run: execvp() for `%s`", descr->name);
            }
        }

        // todo: iterate again and monitor child procs

    }

}