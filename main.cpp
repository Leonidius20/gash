#include <iostream>
#include "lexer/lexer.h"
#include "ast/tree_node.h"
#include "interpreter/interpreter.h"
#include "parser/parser.h"

using namespace std;
using namespace gash;

int main(int argc, char *argv[]) {
    string input;
    if (argc <= 1) { // no arg supplied
        cout << "> ";
        getline(cin, input);
    } else {
        for (int i = 1; i < argc; i++) {
            input += argv[i];
        }
    }

    int result = 127;
    try {
        auto tokens = lexer(input).tokenize();
        auto list_tree_node = parser(std::move(tokens)).parse();
        interpreter i;
        result = list_tree_node->accept(&i);
        cout << "Exit code: " << result << '\n';
    } catch (const invalid_argument &e) {
        cerr << e.what() << '\n';
    }

    return result; // TODO: return result?
}
