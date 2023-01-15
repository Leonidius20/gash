#pragma once

#include <string>

namespace gash {

    class tree_node {

    };

    class command : public tree_node {
        std::string pathname;
    public:
        [[nodiscard]] std::string get_pathname() const { return pathname; };
    };

}
