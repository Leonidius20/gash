#pragma once

#include <vector>
#include "../lexer/token.h"
#include "../ast/tree_node.h"

namespace gash {

    class parser {
    private:
        std::vector< std::unique_ptr<token> > tokens;
        unsigned offset = 0;

        void eatToken(token::type type);

        std::unique_ptr<group_command> parse_list();

        std::unique_ptr<group_command> parse_group_command();

        std::unique_ptr<expression> parse_expression();

        std::unique_ptr<pipeline> parse_pipeline();

        std::unique_ptr<command> parse_command();
    public:
        explicit parser(std::vector< std::unique_ptr<token> > tokens) : tokens(std::move(tokens)) {}

        std::unique_ptr<group_command> parse();
    };

}
