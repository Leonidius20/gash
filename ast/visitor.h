#pragma once

#include <memory>

namespace gash {

    // to avoid circular dependency, make forward declarations
    // instead of including tree_node.h
    class simple_command;
    class pipeline;
    class group_command;
    class or_node;
    class and_node;

    class visitor {
    public:
        virtual int visit(simple_command *node) = 0;
        virtual int visit(pipeline* node) = 0;
        virtual int visit(group_command *node) = 0;
        virtual int visit(or_node *node) = 0;
        virtual int visit(and_node *node) = 0;
    };

}
