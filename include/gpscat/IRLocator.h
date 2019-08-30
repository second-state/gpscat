#pragma once

#include <llvm/IR/Module.h>

namespace gpscat {

class IRLocator {
public:
    void run(llvm::Module *M);
};

} // end namespace gpscat
