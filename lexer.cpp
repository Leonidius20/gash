#include <vector>
#include <memory>
#include "lexer.h"

using namespace std;

namespace gash {

    vector< unique_ptr<token> > lexer::tokenize() {
        vector< unique_ptr<token> > result;

        for (offset = 0; offset < text.size(); offset++) {
            char c = text[offset];

            // identify what the next token will be, judging by its first char (c)
            if (isspace(c)) {
                continue;
            } else if (is_char_part_of_pathname(c)) {
                result.push_back(read_pathname());
            } else if (c == '|') {
                if (text[offset + 1] == '|') {
                    result.push_back(make_unique<or_operator>());
                    offset++;
                } else {
                    result.push_back(make_unique<pipe>());
                }
            } else if (c == '&') {
                if (text[offset + 1] == '&') {
                    result.push_back(make_unique<and_operator>());
                    offset++;
                } else {
                    throw invalid_argument("Unrecognized symbol during tokenizing: single ampersand at offset " + to_string(offset));
                }
            } else if (c == ';') {
                result.push_back(make_unique<semicolon>());
            } else if (c == '{') {
                result.push_back(make_unique<opening_curly_bracket>());
            } else if (c == '}') {
                result.push_back(make_unique<closing_curly_bracket>());
            } else {
                throw invalid_argument("Unrecognized symbol during tokenizing at offset " + to_string(offset));
            }
        }

        return result;
    }

    unique_ptr<token> lexer::read_pathname() {
        // first char of a pathname
        unsigned start_offset = offset;
        // number of chars in a pathname
        unsigned num_chars = 1;

        while (offset + 1 != text.size() &&
                is_char_part_of_pathname(text[offset + 1])) {
            offset++;
            num_chars++;
        }

        return make_unique<pathname>(
                text.substr(start_offset, num_chars));
    }

    bool lexer::is_char_part_of_pathname(char c) {
        return isalnum(c) || c == '.' || c == '/';
    }


}