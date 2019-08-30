#include <gpscat/CoFloCoWrapper.h>
#include <gpscat/Utils.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <system_error>

static llvm::cl::opt<unsigned int> numInlines("inline", llvm::cl::desc("Maximum number of function inline steps"), llvm::cl::init(0));
static llvm::cl::opt<bool> eagerInline("eager-inline", llvm::cl::desc("Exhaustively inline (acyclic call hierarchies only)"));
static llvm::cl::opt<std::string> functionName("function", llvm::cl::desc("Entry function for the cost analysis"), llvm::cl::init(std::string()));

using namespace std::literals;

namespace gpscat {

void CoFloCoWrapper::cost2tick(llvm::Module *M, const BlockCostMapType &blockCostMap) {
    // Rename existing "tick" function
    
    llvm::Function *F = M->getFunction("tick");
    if(F) F->setName("__original_tick_function");

    // Create "tick" dummy function
    llvm::Constant *tickConstant = M->getOrInsertFunction("tick", llvm::Type::getVoidTy(M->getContext()), llvm::Type::getInt32Ty(M->getContext()));
    llvm::Function *tickFunc = llvm::dyn_cast<llvm::Function>(tickConstant);
    assert(tickFunc != nullptr);
    
    for(auto &&F : *M) {
        for(auto &&BB : F) {
            if(const auto& it = blockCostMap.find(&BB); it != blockCostMap.end()) {
                //Notice the hard coded type
                const auto& cost = llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(M->getContext()), it->second);
                llvm::CallInst::Create(tickFunc->getFunctionType(), tickFunc, {cost}, "", BB.getTerminator());
            }
        }
    }
}

void CoFloCoWrapper::extractKoatCostRelationSystem(const std::string &bitcodePath, const std::string &koatCRSPath) {
    // Use llvm2kittel to extract cost relations
    llvm::ErrorOr<std::string> llvm2kittelPath = llvm::sys::findProgramByName("llvm2kittel");
    if(std::error_code ec = llvm2kittelPath.getError()) {
        std::cerr << ec << std::endl
                  << ec.message() << std::endl;
        return;
    }

    std::vector<std::string> args = {"llvm2kittel"s, bitcodePath, "-complexity-tuples"s, "-division-constraint"s, "exact"s, "-increase-strength"s, "-select-is-control"};
    if(numInlines) {
        args.push_back("-inline="s + std::to_string(numInlines));
        args.push_back("-inline-voids");
    }
    if(eagerInline) {
        args.push_back("-eager-inline"s);
    }
    if(!functionName.empty()) {
        args.push_back("-function="s + functionName);
    }

    llvm::sys::ExecuteAndWait(llvm2kittelPath.get(),
                              std::vector<llvm::StringRef>(std::begin(args), std::end(args)),
                              llvm::None,
                              {llvm::Optional<llvm::StringRef>(llvm::None), llvm::Optional<llvm::StringRef>(koatCRSPath), llvm::Optional<llvm::StringRef>(llvm::None)},
                              60);
}

void CoFloCoWrapper::convertToCoFloCoFormat(const std::string &koatCRSPath, const std::string &outputPath) {
    // Step 1: koat2cfg
    llvm::ErrorOr<std::string> koat2cfgPath = llvm::sys::findProgramByName("koat2cfg.pl");
    if(std::error_code ec = koat2cfgPath.getError()) {
        std::cerr << ec << std::endl
                  << ec.message() << std::endl;
        return;
    }

    std::string cfgPath = getTemporaryFilePath("gpscat", "tmp.koat.cfg");
    llvm::sys::ExecuteAndWait(koat2cfgPath.get(),
                              {"koat2cfg.pl"s, koatCRSPath, "tick_cost"s, "-o"s, cfgPath});

    // Step 2: cfg2ces
    llvm::ErrorOr<std::string> cfg2cesPath = llvm::sys::findProgramByName("cfg2ces.pl");
    if(std::error_code ec = cfg2cesPath.getError()) {
        std::cerr << ec << std::endl
                  << ec.message() << std::endl;
        return;
    }

    llvm::sys::ExecuteAndWait(cfg2cesPath.get(),
                              {"cfg2ces.pl"s, cfgPath, "-o"s, outputPath});

    // Remove temporary file
    llvm::sys::fs::remove(cfgPath);
}

void CoFloCoWrapper::extractCostRelationSystem(llvm::Module *M, const std::string &outputPath) {
    std::string bitcodePath = getTemporaryFilePath("gpscat", "tmp.bc");
    writeBitcodeFile(M, bitcodePath);

    std::string koatCRSPath = getTemporaryFilePath("gpscat", "tmp.koat");
    extractKoatCostRelationSystem(bitcodePath, koatCRSPath);

    convertToCoFloCoFormat(koatCRSPath, outputPath);

    // Remove temporary files
    llvm::sys::fs::remove(bitcodePath);
    llvm::sys::fs::remove(koatCRSPath);
}

std::string CoFloCoWrapper::parseCoFloCoOutput(const std::string &path) {
    std::string line;
    std::ifstream inputFile(path);

    // Our goal is to find and parse this line:
    //### Maximum cost of func(arg0, arg1, ...): costBound\n
    while(std::getline(inputFile, line)) {
        if(line.compare(0, 16, "### Maximum cost") == 0) {
            std::string costBound = line.substr(line.find(':') + 2);
            return costBound;
        }
    }

    std::cerr << "Failed to parse CoFloCo output." << std::endl;
    return std::string();
}

std::string CoFloCoWrapper::readCRSAndSolveUpperBound(const std::string &path) {
    llvm::ErrorOr<std::string> coflocoPath = llvm::sys::findProgramByName("cofloco");
    if(std::error_code ec = coflocoPath.getError()) {
        std::cerr << ec << std::endl
                  << ec.message() << std::endl;
        return std::string();
    }

    std::string coflocoOutputPath = getTemporaryFilePath("gpscat", "cofloco");
    llvm::sys::ExecuteAndWait(coflocoPath.get(),
                              {"cofloco"s, "-i"s, path, "-v"s, "0"s, "-compute_lbs"s, "no"s, "-solve_fast"s},
                              llvm::None,
                              {llvm::Optional<llvm::StringRef>(llvm::None), llvm::Optional<llvm::StringRef>(coflocoOutputPath), llvm::Optional<llvm::StringRef>(llvm::None)},
                              60);

    auto &&parsedOutput = parseCoFloCoOutput(coflocoOutputPath);

    // Remove temporary file
    llvm::sys::fs::remove(coflocoOutputPath);

    return parsedOutput;
}

std::string CoFloCoWrapper::removeNat(const std::string &exp) {
    // Replace all occurrences of nat(x) with (x)
    std::string newExp;
    std::size_t pos = 0, lastPos = 0;

    while((pos = exp.find("nat(", pos)) != std::string::npos) {
        newExp += exp.substr(lastPos, pos - lastPos);
        pos += 3; // skip "nat"
        lastPos = pos;
    }

    newExp += exp.substr(lastPos, std::string::npos);

    return newExp;
}

std::string CoFloCoWrapper::replaceNatWithMax(const std::string &exp) {
    // Replace all occurrences of nat(x) with max([x,0])
    auto findClosingParenthesis = [&exp](const std::size_t beginPos) -> std::size_t {
        auto pos = beginPos;
        std::size_t numLeftParenthesis = 0;
        while(pos < exp.size()) {
            numLeftParenthesis += exp[pos] == '(';
            numLeftParenthesis -= exp[pos] == ')';
            if(numLeftParenthesis == 0)
                return pos;
            ++pos;
        }
        return std::string::npos;
    };

    if(std::size_t pos = exp.find("nat("); pos != std::string::npos) {
        auto closingPos = findClosingParenthesis(pos + 3);
        assert(closingPos != std::string::npos);
        return exp.substr(0, pos) +
                "max(["s +
                replaceNatWithMax(exp.substr(pos + 4, closingPos - pos - 4)) +
                ",0])"s +
                replaceNatWithMax(exp.substr(closingPos + 1));
    }

    return exp;
}

} // end namespace gpscat
