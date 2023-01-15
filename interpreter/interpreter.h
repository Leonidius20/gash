#pragma once

#include <memory>
#include "../ast/visitor.h"

namespace gash {

    class interpreter : public visitor {
    public:
        int visit(const std::unique_ptr<simple_command>& node) override;
        int visit(const std::unique_ptr<pipeline>& node) override;
    };

    // value returned if unable to launch program
    const int ERROR_VALUE = 127;

    void try_load_program(const std::string& pathname);
    int monitor_child_process(int pid, const std::string& pathname);

    void printErrorForCommand(const std::string& message, const std::string& pathname, bool useErrno);

}
