#pragma once

#include <memory>
#include "tree_node.h"

namespace gash {

    class visitor {
        virtual int visit(const std::unique_ptr<simple_command>& node) = 0;
        virtual int visit(const std::unique_ptr<pipeline>& node) = 0;
    };

}
