#include <string>
#include <iostream>
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
        //cout << "Entered parse_list()" << '\n';
        vector<unique_ptr<expression>> expressions;

        expressions.push_back(parse_expression());

        while (offset < tokens.size() && tokens[offset]->get_type() == token::type::SEMICOLON) {
            eatToken(token::type::SEMICOLON);
            expressions.push_back(parse_expression());
        }

        //cout << "Exiting parse_list()" << '\n';
        return make_unique<group_command>(std::move(expressions));
    }

    // pipeline ((AND | OR) pipeline)
    std::unique_ptr<expression> parser::parse_expression() {
        //cout << "Entered parse_expression()" << '\n';
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

        //cout << "Exisiting parse_expression()" << '\n';
        return node;
    }

    std::unique_ptr<group_command> parser::parse_group_command() {
        //cout << "Entered parse_group_command()" << '\n';
        eatToken(token::type::OPENING_CURLY_BRACKET);
        std::unique_ptr<group_command> list = parse_list();
        eatToken(token::type::CLOSING_CURLY_BRACKET);
        //cout << "Exiting parse_group_command()" << '\n';
        return std::move(list);
    }

    // command (PIPE command)*
    std::unique_ptr<pipeline> parser::parse_pipeline() {
        //cout << "Entered parse_pipeline()" << '\n';
        vector<unique_ptr<command>> commands;
        commands.push_back(parse_command());

        while (offset < tokens.size() && tokens[offset]->get_type() == token::type::PIPE) {
            eatToken(token::type::PIPE);
            commands.push_back(parse_command());
        }

        //cout << "Exiting parse_pipeline()" << '\n';
        return make_unique<pipeline>(std::move(commands));
    }

    // (simple_cmd | group_cmd)
    std::unique_ptr<command> parser::parse_command() {
       // cout << "Entered parse_command()" << '\n';
        if (tokens[offset]->get_type() == token::type::PATHNAME) {
            auto path = dynamic_cast<pathname*>(tokens[offset].get())->get_text();
            eatToken(token::type::PATHNAME);
           // cout << "Exiting parse_command()" << '\n';
            return make_unique<simple_command>(path);
        } else {
            //cout << "Exiting parse_command()" << '\n';
            return std::move(parse_group_command());
        }
    }

}