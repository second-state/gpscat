#pragma once

#include <gpscat/AssemblyCostModel.h>
#include <gpscat/MappingExtractor.h>

#include <llvm/IR/InstVisitor.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>

#include <unordered_map>
#include <vector>

namespace gpscat {

using InstCostMapType = std::unordered_map<llvm::Instruction*, CostTy>;
using BlockCostMapType = std::unordered_map<llvm::BasicBlock*, CostTy>;

class IRCostCalculator : public llvm::InstVisitor<IRCostCalculator> {
public:
    IRCostCalculator() = delete;
    IRCostCalculator(llvm::Module *M, const AssemblyCostModel &costModel, const IRAsmMapType &IRAsmMap)
                        : costModel(costModel), IRAsmMap(IRAsmMap) {
        visit(M);
    }

    void visitInstruction(llvm::Instruction &I);

    CostTy getInstCost(llvm::Instruction *I) const {
        auto it = instCostMap.find(I);
        return it == instCostMap.end() ? 0 : it->second;
    }
    CostTy getBlockCost(llvm::BasicBlock *BB) const {
        auto it = blockCostMap.find(BB);
        return it == blockCostMap.end() ? 0 : it->second;
    }

    const InstCostMapType &getInstCostMap() const {
        return instCostMap;
    }
    const BlockCostMapType &getBlockCostMap() const {
        return blockCostMap;
    }

    void generateInstCostMetadata(llvm::Module *M) const;
    void generateBlockCostMetadata(llvm::Module *M) const;

private:
    // notice that they are references!
    const AssemblyCostModel &costModel;
    const IRAsmMapType &IRAsmMap;

    InstCostMapType instCostMap;
    BlockCostMapType blockCostMap;
};

} // end namespace gpscat
