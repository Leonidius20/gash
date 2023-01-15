#include <iostream>
#include "lexer/lexer.h"
#include "ast/tree_node.h"
#include "interpreter/interpreter.h"

using namespace std;
using namespace gash;

int main(int argc, char *argv[]) {
    if (argc <= 1) { // no arg supplied
        cout << "> ";
    }

    string input;
    getline(cin, input);

    /*try {
        auto tokens = lexer(input).tokenize();
        cout << tokens.size() << '\n';
        for (auto & token : tokens) {
            cout << token->get_type() << " ";
        }
        cout << '\n';
    } catch (const invalid_argument &e) {
        cerr << e.what() << '\n';

    }*/

    auto cmd = make_unique<simple_command>(input);

    interpreter inte;
    int res = inte.visit(cmd);
    cout << res << '\n';

    return 0; // TODO: return result?
}
