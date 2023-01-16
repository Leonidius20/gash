#pragma once

#include <memory>
#include "tree_node.h"

namespace gash {

    class visitor {
    public:
        virtual int visit(simple_command *node) = 0;
        virtual int visit(pipeline* node) = 0;
        virtual int visit(group_command *node) = 0;
        virtual int visit(or_node *node) = 0;
        virtual int visit(and_node *node) = 0;
    };

}
