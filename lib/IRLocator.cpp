#include <gpscat/IRLocator.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/IR/IntrinsicInst.h>

#include <opt/Debugify.h>

namespace gpscat {

void IRLocator::run(llvm::Module *M) {
    llvm::legacy::PassManager PM;

    // The debugify pass does exactly what we want
    // Before debugify pass, we need to strip debug symbols
    PM.add(llvm::createStripSymbolsPass(true));
    PM.add(createDebugifyModulePass());

    PM.run(*M);

    // remove debug intrinsics
    for(auto &F : *M) {
        for(auto &BB : F) {
            for(auto II = BB.begin(); II != BB.end();) {
                auto &I = *II++;
                if(llvm::isa<llvm::DbgInfoIntrinsic>(&I)) {
                    I.eraseFromParent();
                }
            }
        }
    }
}

} // end namespace gpscat
