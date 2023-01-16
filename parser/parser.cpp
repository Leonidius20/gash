#include <string>
#include "parser.h"

using namespace std;

namespace gash {

    std::unique_ptr<group_command> parser::parse() {
        return parse_list();
    }

    void parser::eatToken(token::type type) {
        if (offset >= tokens.size()) {
            string what = "Syntax error at offset ";
            what.append(to_string(offset));
            what.append(": unexpected end of file, expected token #");
            what.append(to_string(type));
            throw invalid_argument(what);
        }
        if (tokens[offset]->get_type() != type) {
            string what = "Syntax error at offset ";
            what.append(to_string(offset));
            what.append(": unexpected token, expected token #");
            what.append(to_string(type));
            throw invalid_argument(what);
        } else offset++;
    }

    // expression (SEMICOLON expression)*
    std::unique_ptr<group_command> parser::parse_list() {
        vector<unique_ptr<expression>> expressions;

        expressions.push_back(parse_expression());

        while (offset < tokens.size() && tokens[offset]->get_type() == token::type::SEMICOLON) {
            eatToken(token::type::SEMICOLON);
            expressions.push_back(parse_expression());
        }

        return make_unique<group_command>(std::move(expressions));
    }

    // pipeline ((AND | OR) pipeline)
    std::unique_ptr<expression> parser::parse_expression() {
        unique_ptr<expression> node = parse_pipeline();

        while (offset < tokens.size()
            && (tokens[offset]->get_type() == token::type::AND_OPERATOR
                || tokens[offset]->get_type() == token::type::OR_OPERATOR)) {

            if (tokens[offset]->get_type() == token::type::AND_OPERATOR) {
                eatToken(token::type::AND_OPERATOR);
                node = make_unique<and_node>(std::move(node), parse_pipeline());
            } else {
                eatToken(token::type::OR_OPERATOR);
                node = make_unique<or_node>(std::move(node), parse_pipeline());
            }
        }

        return node;
    }

    std::unique_ptr<group_command> parser::parse_group_command() {
        eatToken(token::type::OPENING_CURLY_BRACKET);
        auto list = parse_list();
        eatToken(token::type::CLOSING_CURLY_BRACKET);
        return list;
    }

    // command (PIPE command)*
    std::unique_ptr<pipeline> parser::parse_pipeline() {
        vector<unique_ptr<command>> commands;
        commands.push_back(parse_command());

        while (offset < tokens.size() && tokens[offset]->get_type() == token::type::PIPE) {
            eatToken(token::type::PIPE);
            commands.push_back(parse_command());
        }

        return make_unique<pipeline>(std::move(commands));
    }

    // (simple_cmd | group_cmd)
    std::unique_ptr<command> parser::parse_command() {
        if (tokens[offset]->get_type() == token::type::PATHNAME) {
            auto path = dynamic_cast<pathname*>(tokens[offset].get())->get_text();
            eatToken(token::type::PATHNAME);
            return make_unique<simple_command>(path);
        } else {
            return parse_group_command();
        }
    }

}