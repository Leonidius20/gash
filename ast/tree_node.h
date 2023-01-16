#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include "visitor.h"

namespace gash {

    class tree_node {
    public:
        virtual int accept(visitor *visitor) = 0;
    };

    class command : public tree_node {
         // todo: virt. method to apply cin and cout from other commands
    };

    class simple_command : public command {
        std::string pathname;
    public:
        explicit simple_command(std::string path) : pathname(std::move(path)) {};

        [[nodiscard]] std::string get_pathname() const { return pathname; };

        int accept(visitor *visitor) override {
            return visitor->visit(this);
        };
    };

    // can be a pipeline or an operator, It's and abstract class
    class expression : public tree_node {
    public:

    };

    // and / or
    class operator_node : public expression {
    private:
        std::unique_ptr<expression> left_child;
        std::unique_ptr<expression> right_child;
    public:
        operator_node(std::unique_ptr<expression> left,
        std::unique_ptr<expression> right)
        : left_child(std::move(left)),
        right_child(std::move(right)) {};

        [[nodiscard]] const std::unique_ptr<expression>& get_left() const { return left_child; };
        [[nodiscard]] const std::unique_ptr<expression>& get_right() const { return right_child; };
    };

    class and_node : public operator_node {
        // children can be other operators or pipelines (aka, expressions)
        // visit()
    public:
        and_node(std::unique_ptr<expression> left,
        std::unique_ptr<expression> right)
        : operator_node(std::move(left), std::move(right)) {};

        int accept(visitor *visitor) override {
            return visitor->visit(this);
        };
    };

    class or_node : public operator_node {
    public:
        or_node(std::unique_ptr<expression> left,
        std::unique_ptr<expression> right)
        : operator_node(std::move(left), std::move(right)) {};

        int accept(visitor *visitor) override {
            return visitor->visit(this);
        };
    };

    // same as list
    class group_command : public command {
    private:
        std::vector< std::unique_ptr<expression> > expressions;
    public:
        explicit group_command(std::vector< std::unique_ptr<expression> > exprs)
                : expressions(std::move(exprs)) {};

        [[nodiscard]] const std::vector< std::unique_ptr<expression> >& get_expressions() const {
            return expressions;
        };

        int accept(visitor *visitor) override {
            return visitor->visit(this);
        };
    };

    class pipeline : public expression {
    private:
        std::vector< std::unique_ptr<command> > commands;
    public:
        explicit pipeline(std::vector< std::unique_ptr<command> > cmds) : commands(std::move(cmds)) {};

        [[nodiscard]] const std::vector< std::unique_ptr<command> >& get_commands() const {
            return commands;
        };

        int accept(visitor *visitor) override {
            return visitor->visit(this);
        };
    };

}
