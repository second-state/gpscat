#include <gpscat/IRCostCalculator.h>

#include <llvm/IR/Instruction.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Metadata.h>

#include <iostream>

namespace gpscat {

void IRCostCalculator::visitInstruction(llvm::Instruction &I) {
    const auto& it = IRAsmMap.find(&I);
    if(it == IRAsmMap.end()) {
        //std::cerr << "Cannot find corresponding asm instructions of " << &I << ' ' << I.getOpcodeName() << std::endl;
        return;
    }

    const auto& asmInstSet = it->second;

    for(const auto& asmInst : asmInstSet) {
        auto cost = costModel.get(asmInst.getName());
        instCostMap[&I] += cost;
        blockCostMap[I.getParent()] += cost;
    }
}

void IRCostCalculator::generateInstCostMetadata(llvm::Module *M) const {
    for(auto &&F : *M) {
        for(auto &&BB : F) {
            for(auto &&I : BB) {
                // Query instruction cost
                const auto& it = instCostMap.find(&I);
                auto cost = it == instCostMap.end() ? 0 : it->second;

                // Beware of the hard-coded integer type
                const auto& costAsConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(I.getContext()), cost);
                llvm::MDNode* costAsMD = llvm::MDNode::get(I.getContext(), llvm::ConstantAsMetadata::get(costAsConstant));

                I.setMetadata("cost", costAsMD);
            }
        }
    }
}

void IRCostCalculator::generateBlockCostMetadata(llvm::Module *M) const {
    for(auto &&F : *M) {
        for(auto &&BB : F) {
            // Query block cost
            const auto& it = blockCostMap.find(&BB);
            auto cost = it == blockCostMap.end() ? 0 : it->second;

            // Beware of the hard-coded integer type
            const auto& costAsConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(BB.getContext()), cost);
            llvm::MDNode* costAsMD = llvm::MDNode::get(BB.getContext(), llvm::ConstantAsMetadata::get(costAsConstant));

            // Attach cost metadata to the terminator instruction of the basic block
            BB.getTerminator()->setMetadata("cost", costAsMD);
        }
    }
}

} // end namespace gpscat
