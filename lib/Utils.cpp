#include <llvm/IR/Module.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/FileSystem.h>

#include <vector>
#include <string>
#include <system_error>
#include <sstream>
#include <iterator>

namespace gpscat {

std::string getTemporaryFilePath(const std::string &prefix, const std::string &suffix) {
    llvm::SmallVector<char, 128> tmpPath;
    llvm::sys::fs::createTemporaryFile(prefix, suffix, tmpPath);
    return std::string(tmpPath.begin(), tmpPath.end());
}

void writeBitcodeFile(llvm::Module *M, const std::string &path) {
    std::error_code ec;
    llvm::raw_fd_ostream tmpFileStream(path, ec);

    llvm::WriteBitcodeToFile(*M, tmpFileStream);
}

std::vector<std::string> split(const std::string &str) {
    std::istringstream iss(str);
    return std::vector<std::string>(std::istream_iterator<std::string>(iss),
                                    std::istream_iterator<std::string>());
}

} // end namespace gpscat
