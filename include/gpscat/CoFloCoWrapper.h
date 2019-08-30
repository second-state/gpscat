#pragma once

#include <gpscat/IRCostCalculator.h>

#include <llvm/IR/Module.h>

#include <string>

namespace gpscat {

class CoFloCoWrapper {
public:
    void cost2tick(llvm::Module *M, const BlockCostMapType &blockCostMap);

    void extractKoatCostRelationSystem(const std::string &bitcodePath, const std::string &koatCRSPath);
    void convertToCoFloCoFormat(const std::string &koatCRSPath, const std::string &outputPath);
    void extractCostRelationSystem(llvm::Module *M, const std::string &outputPath);

    std::string parseCoFloCoOutput(const std::string &path);
    std::string readCRSAndSolveUpperBound(const std::string &CRSFilePath);

    std::string removeNat(const std::string &exp);
    std::string replaceNatWithMax(const std::string &exp);
};

} // end namespace gpscat
