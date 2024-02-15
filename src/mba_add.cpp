//
// Created by suz305 on 2/26/19.
//

#ifndef MBA_ADD_CPP
#define MBA_ADD_CPP
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SCCIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <cstddef>
#include <iostream>
// #include <z3++.h>
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include <llvm-16/llvm/IR/PassManager.h>
#include <unordered_map>

namespace {
class MbaAddPass : public llvm::PassInfoMixin<MbaAddPass> {
public:
  static bool isRequired() { return true; }
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
    bool changed = false;
      for (auto &BB : F) {
        for (auto I = BB.begin(), end = BB.end(); I != end; ++I) {
          I->print(llvm::errs());
          std::cout << "1 " << std::endl;
          auto *op = llvm::dyn_cast<llvm::BinaryOperator>(I);
          if (!op)
            continue;
          if (op->getOpcode() != llvm::Instruction::Add)
            continue;
          std::cout << 2 << std::endl;
          llvm::IRBuilder<> Builder(op);
          auto and_op = Builder.CreateAnd(op->getOperand(0), op->getOperand(1));
          auto or_op = Builder.CreateOr(op->getOperand(0), op->getOperand(1));
          auto new_add = Builder.CreateAdd(op->getOperand(0), op->getOperand(1));
        //   llvm::BinaryOperator::Create(llvm::Instruction::Add,
        //                                               and_op, or_op);
          llvm::ReplaceInstWithValue(I, new_add);
          changed = true;
          std::cout << 3 << std::endl;
        }
      }

    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }
  //   llvm::PreservedAnalyses run(llvm::Function &F,
  //                               llvm::FunctionAnalysisManager &AM) {
  //     bool changed = false;
  //     // std::cout << "running MyPeepholeAddPass\n";
  //     std::vector<llvm::Instruction *> to_remove;

  //     for (auto *I : to_remove) {
  //       // std::cout << "erasing instruction\n";
  //       I->eraseFromParent();
  //       changed = true;
  //     }
  //     // std::cout << "finished MyPeepholeAddPass\n";
  //     return changed ? llvm::PreservedAnalyses::none()
  //                    : llvm::PreservedAnalyses::all();
  //   }
};

} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  //   std::cout << "llvmGetPassPluginInfo hook\n";
  return {
      LLVM_PLUGIN_API_VERSION, "MbaAddPass", LLVM_VERSION_STRING,
      [](llvm::PassBuilder &PB) {
        // PB.registerVectorizerStartEPCallback(
        // [](llvm::ModulePassManager &PM, OptimizationLevel Level) {
        //   PM.addPass(MainPass());
        // });
        PB.registerVectorizerStartEPCallback(
            [](llvm::FunctionPassManager &PM, llvm::OptimizationLevel Level) {
              PM.addPass(MbaAddPass());
            });
        PB.registerPipelineParsingCallback(
            [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
               llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
              //   std::cout << "in llvmGetPassPluginInfo callback, current pass
              //   name: " << Name.str() << "\n";
              if (Name == "mba-add") {
                FPM.addPass(MbaAddPass());
                return true;
              }
              return false;
            });
      }};
}

#endif // MBA_ADD_CPP
