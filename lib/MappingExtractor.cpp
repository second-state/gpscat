#include <gpscat/MappingExtractor.h>
#include <gpscat/IRLocator.h>
#include <gpscat/AssemblyInstruction.h>
#include <gpscat/AssemblyCostModel.h>
#include <gpscat/Utils.h>

#include <llvm/IR/DebugLoc.h>

#include <string>
#include <utility>
#include <iostream>
#include <fstream>

namespace gpscat {

IRIDAsmMapType MappingExtractor::extractMapping(const std::string &path, const AssemblyCostModel &costModel) {
    IRIDAsmMapType IRIDAsmMap;

    InstructionID currentLocLineno = 0;
    std::string line;

    std::ifstream inputFile(path);
    std::size_t lineNum = 0;
    while(std::getline(inputFile, line)) {
        ++lineNum;
        const auto& tokens = split(line);
        
        if(tokens.empty())
            continue;

        // .loc directives
        // .loc fileno lineno [column] [options]
        // lineno == 0 indicates that no source line has been specified accoring to DWARF.
        if(tokens[0] == ".loc" && tokens.size() >= 3) {
            // Beware of the hardcoded type
            currentLocLineno = std::stoul(tokens[2]);
        }
        // known assembly instructions
        else if(costModel.hasInst(tokens[0])) {
            if(currentLocLineno == 0)
                std::cerr << "No corresponding LLVM IR instruction for Assembly instruction: "
                          << lineNum << "\t" << tokens[0] << std::endl;
            IRIDAsmMap[currentLocLineno].push_back(AssemblyInstruction(tokens[0]));
        }
    }
    return IRIDAsmMap;
}

IRAsmMapType MappingExtractor::extractMapping(const std::string &path, const AssemblyCostModel &costModel, llvm::Module* M) {
    IRIDAsmMapType IRIDAsmMap = extractMapping(path, costModel);

    InstIDPtrMapType instIDPtrMap;
    for(auto& F : *M) {
        for(auto& BB : F) {
            for(auto& I : BB) {
                if(const llvm::DebugLoc &loc = I.getDebugLoc()) {
                    instIDPtrMap[loc.getLine()] = &I;
                }
            }
        }
    }

    IRAsmMapType IRAsmMap;
    for(const auto& [IRID, Asms] : IRIDAsmMap) {
        if(const auto& it = instIDPtrMap.find(IRID); it != instIDPtrMap.end())
            IRAsmMap[it->second] = std::move(Asms);
        else if(IRID != 0)
            std::cerr << "Cannot find corresponding instruction ID : " << IRID << std::endl;
    }
    return IRAsmMap;
}

} // end namespace gpscat
