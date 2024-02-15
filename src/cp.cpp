//
// Created by suz305 on 2/26/19.
//

#ifndef PH_ADD_CPP
#define PH_ADD_CPP
#include "llvm/Pass.h"
#include "llvm/Support/MemoryBuffer.h"

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
#include <llvm-16/llvm/IR/BasicBlock.h>
#include <llvm-16/llvm/IR/Constant.h>
#include <llvm-16/llvm/IR/Value.h>
#include <queue>
#include <unordered_map>
#include <variant>

namespace {

// a dense constant propagation pass that handles load and store instructions
// with constant addresses
class MyConstantPropagation
    : public llvm::PassInfoMixin<MyConstantPropagation> {
public:
  static bool isRequired() { return true; }
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM) {
    bool changed = false;
    // std::cout << "running MyPeepholeAddPass\n";
    // std::vector<llvm::Instruction*> to_remove;
    std::queue<llvm::BasicBlock *> worklist;
    // map both alloca address and load instruction to their constant value
    std::unordered_map<llvm::Value *, Lattice> constantMap;
    std::unordered_set<llvm::Value *> inWorklist;
    worklist.push(&F.getEntryBlock());
    while (!worklist.empty()) {
      auto *BB = worklist.front();
      bool bb_changed = false;
      worklist.pop();
      inWorklist.erase(BB);
      for (auto &I : *BB) {
        // match:
        // %0 = alloca ...
        // %1 = load %0
        // update constantMap[%1] with constantMap[%0]
        llvm::outs() << "I: " << I << "\n";
        if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I)) {
          auto *ptr = loadInst->getPointerOperand();
          auto *allocaPtr = llvm::dyn_cast<llvm::AllocaInst>(ptr);
          if (!allocaPtr)
            continue;
          if (constantMap.find(loadInst) == constantMap.end()) {
            constantMap[loadInst] = LatticeState::BOTTOM;
          }
          if (constantMap.find(allocaPtr) != constantMap.end()) {
            auto lattice = constantMap[loadInst];
            auto new_val = join(lattice, constantMap[allocaPtr]);
            if (new_val != lattice) {
              llvm::outs() << "updating constantMap[" << *loadInst << "] with "
                           << *allocaPtr << "\n";
              constantMap[loadInst] = new_val;
              bb_changed = true;
            }
          }
        } else if (auto *storeInst = llvm::dyn_cast<llvm::StoreInst>(&I)) {
          // match: store %valOp, ptr %ptrOp
          // update constantMap[ptrOp] with valOp or constantMap[valOp]
          auto *ptrOp = storeInst->getPointerOperand();
          auto *valOp = storeInst->getValueOperand();
          if (constantMap.find(ptrOp) == constantMap.end()) {
            constantMap[ptrOp] = LatticeState::BOTTOM;
          }
          if (auto *constValOp = llvm::dyn_cast<llvm::Constant>(valOp)) {
            auto new_val = join(constantMap[ptrOp],
                                llvm::dyn_cast<llvm::Value>(constValOp));
            llvm::outs() << "new_val: "
                         << (std::holds_alternative<LatticeState>(new_val)
                                 ? "lattice"
                                 : "inst")
                         << "\n";
            if (new_val != constantMap[ptrOp]) {
              llvm::outs() << "updating constantMap[" << *ptrOp << "] with "
                           << *valOp << "\n";
              constantMap[ptrOp] = new_val;
              bb_changed = true;
            }
          } else if (constantMap.find(valOp) != constantMap.end()) {
            auto new_val = join(constantMap[ptrOp], constantMap[valOp]);
            llvm::outs() << "new_val: "
                         << (std::holds_alternative<LatticeState>(new_val)
                                 ? "lattice"
                                 : "inst")
                         << "\n";
            if (new_val != constantMap[ptrOp]) {
              llvm::outs() << "updating constantMap[" << *ptrOp << "] with "
                           << *valOp << "\n";
              constantMap[ptrOp] = new_val;
              bb_changed = true;
            }
          }
        }
      }
      if (bb_changed) {
        for (const auto &[key, value] : constantMap) {
          llvm::outs() << "constantMap[" << *key << "]: ";
          if (std::holds_alternative<LatticeState>(value)) {
            llvm::outs() << "LatticeState\n";
          } else {
            llvm::outs() << *std::get<llvm::Value *>(value) << "\n";
          }
        }
        for (auto *succ : llvm::successors(BB)) {
          if (inWorklist.find(succ) == inWorklist.end()) {
            worklist.push(succ);
            inWorklist.insert(succ);
          }
        }
      }
    }
    llvm::outs() << "rewrite load instructions\n";
    for (auto &BB : F) {
      for (auto &I : make_early_inc_range(BB)) {
      // for (auto I = BB.begin(), end = BB.end(); I != end; ++I) {
        if (auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(&I)) {
          llvm::outs() << "loadInst: " << *loadInst << "\n";
          auto map_ind = constantMap.find(loadInst);
          if (map_ind == constantMap.end())
            continue;
          llvm::outs() << "found loadInst in constantMap\n";
          auto lattice = map_ind->second;
          if (std::holds_alternative<llvm::Value*>(lattice)) {
            llvm::outs() << "lattice holds llvm::Value*\n";
            auto *constVal = std::get<llvm::Value *>(lattice);
            if (!constVal)
              continue;
            llvm::outs() << "replacing loadInst with " << *constVal << "\n";
            loadInst->replaceAllUsesWith(constVal);
            loadInst->eraseFromParent();
            changed = true;
          }
        }
      }
    }

    // std::cout << "finished MyPeepholeAddPass\n";
    return changed ? llvm::PreservedAnalyses::none()
                   : llvm::PreservedAnalyses::all();
  }

private:
  enum class LatticeState {
    BOTTOM,
    TOP,
  };
  using Lattice = std::variant<LatticeState, llvm::Value *>;
  static Lattice join(const Lattice &l1, const Lattice &l2) {
    llvm::outs() << "l1 type: ";
    if (std::holds_alternative<LatticeState>(l1)) {
      llvm::outs() << "LatticeState\n";
    } else {
      // Assuming l1 holds an llvm::Value* here
      llvm::Value *value = std::get<llvm::Value *>(l1);
      if (value) {
        value->print(llvm::outs());
      }
      llvm::outs() << "\n";
    }

    llvm::outs() << "l2 type: ";
    if (std::holds_alternative<LatticeState>(l2)) {
      llvm::outs() << "LatticeState\n";
    } else {
      // Assuming l1 holds an llvm::Value* here
      llvm::Value *value = std::get<llvm::Value *>(l2);
      if (value) {
        value->print(llvm::outs());
      }
      llvm::outs() << "\n";
    }

    if (std::holds_alternative<LatticeState>(l1)) {
      llvm::outs() << "l1 is LatticeState\n";
      if (std::get<LatticeState>(l1) == LatticeState::TOP) {
        llvm::outs() << "l1 is TOP\n";
        return LatticeState::TOP;
      } else if (std::get<LatticeState>(l1) == LatticeState::BOTTOM) {
        llvm::outs() << "l1 is BOTTOM\n";
        return l2;
      }
    }
    if (std::holds_alternative<LatticeState>(l2)) {
      llvm::outs() << "l2 is LatticeState\n";
      if (std::get<LatticeState>(l2) == LatticeState::TOP) {
        llvm::outs() << "l2 is TOP\n";
        return LatticeState::TOP;
      } else if (std::get<LatticeState>(l2) == LatticeState::BOTTOM) {
        llvm::outs() << "l2 is BOTTOM\n";
        return l1;
      }
    }
    llvm::outs() << "both should hold llvm::Value*\n";
    auto *v1 = std::get<llvm::Value *>(l1);
    auto *v2 = std::get<llvm::Value *>(l2);
    if (v1 == v2) {
      return l1;
    } else {
      return LatticeState::TOP;
    }
  }
};
} // namespace

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  //   std::cout << "llvmGetPassPluginInfo hook\n";
  return {
      LLVM_PLUGIN_API_VERSION, "MyConstProp", LLVM_VERSION_STRING,
      [](llvm::PassBuilder &PB) {
        // PB.registerVectorizerStartEPCallback(
        // [](llvm::ModulePassManager &PM, OptimizationLevel Level) {
        //   PM.addPass(MainPass());
        // });
        PB.registerVectorizerStartEPCallback(
            [](llvm::FunctionPassManager &PM, llvm::OptimizationLevel Level) {
              PM.addPass(MyConstantPropagation());
            });
        PB.registerPipelineParsingCallback(
            [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
               llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
              //   std::cout << "in llvmGetPassPluginInfo callback, current pass
              //   name: " << Name.str() << "\n";
              if (Name == "my-cp") {
                FPM.addPass(MyConstantPropagation());
                return true;
              }
              return false;
            });
      }};
}

#endif // PH_ADD_CPP
