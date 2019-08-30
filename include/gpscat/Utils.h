#pragma once

#include <llvm/IR/Module.h>

#include <vector>
#include <string>

namespace gpscat {

std::string getTemporaryFilePath(const std::string &prefix, const std::string &suffix);

void writeBitcodeFile(llvm::Module *M, const std::string &path);

std::vector<std::string> split(const std::string &str);

} // end namespace gpscat
