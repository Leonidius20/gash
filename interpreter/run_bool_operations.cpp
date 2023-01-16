#include <cstring>
#include "interpreter.h"
#include "../ast/tree_node.h"

using namespace std;

namespace gash {

    int interpreter::visit(or_node *node) {
        int last_return_code = node->get_left()->accept(this);
        if (last_return_code == 0) {
            last_return_code = node->get_right()->accept(this);
        }
        return last_return_code;
    }

    int interpreter::visit(and_node *node) {
        int last_return_code = node->get_left()->accept(this);
        if (last_return_code != 0) {
            last_return_code = node->get_right()->accept(this);
        }
        return last_return_code;
    }

}