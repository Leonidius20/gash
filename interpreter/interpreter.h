#pragma once

#include <memory>
#include "../ast/visitor.h"

namespace gash {

    class interpreter : public visitor {
    public:
        int visit(simple_command *node) override;
        int visit(pipeline *node) override;
        int visit(group_command *node) override;
        int visit(or_node *node) override;
        int visit(and_node *node) override;
    };

    // value returned if unable to launch program
    const int ERROR_VALUE = 127;

    void try_load_program(const std::string& pathname);
    int monitor_child_process(int pid, const std::string& pathname);

    void printErrorForCommand(const std::string& message, const std::string& pathname, bool useErrno);
    void printErrorForPipeline(const std::string& message, bool useErrno);

}
