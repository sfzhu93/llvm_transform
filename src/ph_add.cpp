//
// Created by suz305 on 2/26/19.
//

#ifndef PH_ADD_CPP
#define PH_ADD_CPP
#include "llvm/Pass.h"
#include "llvm/Support/MemoryBuffer.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/ADT/PostOrderIterator.h"

#include <cstddef>
#include <iostream>
//#include <z3++.h>
#include <unordered_map>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace {
class MyPeepholeAddPass : public llvm::PassInfoMixin<MyPeepholeAddPass> {
public:
    static bool isRequired() { return true; }
    llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM){
        bool changed = false;
        // std::cout << "running MyPeepholeAddPass\n";
        std::vector<llvm::Instruction*> to_remove;
        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *op = llvm::dyn_cast<llvm::BinaryOperator>(&I)) {
                    if (op->getOpcode() == llvm::Instruction::Add) {
                        // std::cout << "found add instruction\n";
                        if (auto *op1 = llvm::dyn_cast<llvm::ConstantInt>(op->getOperand(0))) {
                            llvm::Value *op2 = op->getOperand(1);
                            if (op1->isZero()) {
                                op->replaceAllUsesWith(op2);
                                // op->eraseFromParent();
                                to_remove.push_back(op);
                            }
                            // if (op1->isZero() && op2 != nullptr) {
                            //     std::vector<llvm::Use *> usesToReplace;
                            //     for (llvm::Use &U : op->uses()) {
                            //         usesToReplace.push_back(&U);
                            //     }
                            //     for (llvm::Use *U : usesToReplace) {
                            //         llvm::User *user = U->getUser();
                            //         user->setOperand(U->getOperandNo(), op2);
                            //     }
                            // }

                        } else if (auto *op2 = llvm::dyn_cast<llvm::ConstantInt>(op->getOperand(1))) {
                            llvm::Value *op1 = op->getOperand(0);
                            if (op2->isZero()) {
                                op->replaceAllUsesWith(op1);
                                // op->eraseFromParent();
                                to_remove.push_back(op);
                            }
                            // if (op2->isZero() && op1 != nullptr) {
                            //     std::vector<llvm::Use *> usesToReplace;
                            //     for (llvm::Use &U : op->uses()) {
                            //         usesToReplace.push_back(&U);
                            //     }
                            //     for (llvm::Use *U : usesToReplace) {
                            //         llvm::User *user = U->getUser();
                            //         user->setOperand(U->getOperandNo(), op1);
                            //     }
                            // }
                            // to_remove.push_back(op);
                        }
                    }
                }
            }
        }

        for (auto *I : to_remove) {
            // std::cout << "erasing instruction\n";
            I->eraseFromParent();
            changed = true;
        }
        // std::cout << "finished MyPeepholeAddPass\n";
        return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
    }
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
//   std::cout << "llvmGetPassPluginInfo hook\n";
  return {LLVM_PLUGIN_API_VERSION, "MyPeepholeAddPass", LLVM_VERSION_STRING,
          [](llvm::PassBuilder &PB) {
            // PB.registerVectorizerStartEPCallback(
                // [](llvm::ModulePassManager &PM, OptimizationLevel Level) {
                //   PM.addPass(MainPass());
                // });
            PB.registerVectorizerStartEPCallback(
                [](llvm::FunctionPassManager &PM, llvm::OptimizationLevel Level) {
                  PM.addPass(MyPeepholeAddPass());
                });
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                //   std::cout << "in llvmGetPassPluginInfo callback, current pass name: " << Name.str() << "\n";
                  if (Name == "my-peephole-add") {
                    FPM.addPass(MyPeepholeAddPass());
                    return true;
                  }
                  return false;
                });
          }};
}

#endif //PH_ADD_CPP
