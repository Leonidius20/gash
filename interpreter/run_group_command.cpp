#include <cstring>
#include "interpreter.h"
#include "../ast/tree_node.h"

using namespace std;

namespace gash {

    int interpreter::visit(group_command *node) {
        int last_return_code;
        for (auto const &expr : node->get_expressions()) {
            last_return_code = expr->accept(this);
        }
        return last_return_code;
    }

}