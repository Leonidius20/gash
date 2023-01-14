#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "token.h"

namespace gash {

    class lexer {
    private:
        std::string text;
        unsigned offset = 0;

    public:
        explicit lexer(std::string input) : text(std::move(input)) {}

        std::vector< std::unique_ptr<token> > tokenize();

    private:
        [[nodiscard]] std::unique_ptr<token> read_pathname();

        [[nodiscard]] static bool is_char_part_of_pathname(char c);
    };

}
