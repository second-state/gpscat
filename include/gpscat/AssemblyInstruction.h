#pragma once

#include <string>

namespace gpscat {

class AssemblyInstruction {
public:
    AssemblyInstruction(const std::string& opcodeName) : opcodeName(opcodeName) {};

    const std::string& getName() const {
        return opcodeName;
    }

private:
    std::string opcodeName;
};

} // end namespace gpscat
