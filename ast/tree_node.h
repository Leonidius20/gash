#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>

namespace gash {

    class tree_node {

    };

    class command : public tree_node {
         // virt. method to apply cin and cout from other commands
    };

    class simple_command : public command {
        std::string pathname;
    public:
        explicit simple_command(std::string path) : pathname(std::move(path)) {};

        [[nodiscard]] std::string get_pathname() const { return pathname; };
    };

    // can be a pipeline or an operator, It's and abstract class
    class expression : public tree_node {

    };

    // and / or
    class operator_node : public expression {
    private:
        std::unique_ptr<expression> left_child;
        std::unique_ptr<expression> right_child;
    public:
        explicit operator_node(std::unique_ptr<expression> left,
        std::unique_ptr<expression> right)
        : left_child(std::move(left)),
        right_child(std::move(right)) {};

        [[nodiscard]] const std::unique_ptr<expression>& get_left() const { return left_child; };
        [[nodiscard]] const std::unique_ptr<expression>& get_right() const { return right_child; };
    };

    class and_node : public operator_node {
        // children can be other operators or pipelines (aka, expressions)
        // visit()
    };

    class or_node : public operator_node {

    };

    // same as list
    class group_command : public command {
    private:
        std::vector< std::unique_ptr<expression> > expressions;
    public:
        explicit group_command(std::vector< std::unique_ptr<expression> > exprs)
                : expressions(std::move(exprs)) {};

        [[nodiscard]] const std::vector< std::unique_ptr<expression> >& getExpressions() const {
            return expressions;
        };
    };

    class pipeline : public expression {
    private:
        std::vector< std::unique_ptr<command> > commands;
    public:
        explicit pipeline(std::vector< std::unique_ptr<command> > cmds) : commands(std::move(cmds)) {};

        [[nodiscard]] const std::vector< std::unique_ptr<command> >& getCommands() const {
            return commands;
        };
    };

}
