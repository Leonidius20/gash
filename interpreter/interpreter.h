#pragma once

#include <memory>
#include "../ast/visitor.h"

namespace gash {

    class interpreter : public visitor {
    public:
        int visit(const std::unique_ptr<command>& node) override;
    };

    void printErrorForCommand(const std::string& message, const std::string& pathname, bool useErrno);

}
