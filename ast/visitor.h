#pragma once

#include <memory>
#include "tree_node.h"

namespace gash {

    class visitor {
        virtual int visit(const std::unique_ptr<command>& node) = 0;
    };

}
