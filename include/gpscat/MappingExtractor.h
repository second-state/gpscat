#pragma once

#include <gpscat/AssemblyCostModel.h>
#include <gpscat/AssemblyInstruction.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Instruction.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace gpscat {

using InstructionID = std::uint32_t;
using InstIDPtrMapType = std::unordered_map<InstructionID, llvm::Instruction*>;

using IRIDAsmMapType = std::unordered_map<InstructionID, std::vector<AssemblyInstruction>>;
using IRAsmMapType = std::unordered_map<llvm::Instruction*, std::vector<AssemblyInstruction>>;

class MappingExtractor {
public:
    IRIDAsmMapType extractMapping(const std::string &path, const AssemblyCostModel &costModel);
    IRAsmMapType extractMapping(const std::string &path, const AssemblyCostModel &costModel, llvm::Module* M);
};

} // end namespace gpscat
