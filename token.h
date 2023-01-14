#pragma once

#include <string>

namespace gash {

    class token {
    public:

        enum type {
            PATHNAME,              // 0
            PIPE,                  // 1
            SEMICOLON,             // 2
            AND_OPERATOR,          // 3
            OR_OPERATOR,           // 4
            OPENING_CURLY_BRACKET, // 5
            CLOSING_CURLY_BRACKET, // 6
        };

        [[nodiscard]] virtual type get_type() const = 0;

    };

    class pathname : public token {
    private:
        const std::string text;
    public:
        explicit pathname(std::string _text) : text(std::move(_text)) {};

        [[nodiscard]] std::string get_text() const { return text; };

        [[nodiscard]] type get_type() const override { return type::PATHNAME; };
    };

    class pipe: public token {
    public:
        [[nodiscard]] type get_type() const override { return type::PIPE; };

    };

    class semicolon : public token {
    public:
        [[nodiscard]] type get_type() const override { return type::SEMICOLON; };
    };

    class or_operator : public token {
    public:
        [[nodiscard]] type get_type() const override { return type::OR_OPERATOR; };
    };

    class and_operator : public token {
    public:
        [[nodiscard]] type get_type() const override { return type::AND_OPERATOR; };
    };

    class opening_curly_bracket : public token {
    public:
        [[nodiscard]] type get_type() const override { return type::OPENING_CURLY_BRACKET; };
    };

    class closing_curly_bracket : public token {
    public:
        [[nodiscard]] type get_type() const override { return type::CLOSING_CURLY_BRACKET   ; };
    };

}
