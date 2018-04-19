#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"        // To print error messages.
#include "llvm/ADT/Statistic.h"        // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"      // To use the iterator instructions(f)
#include "llvm/IR/Instructions.h"      // To have access to the Instructions.
#include "llvm/IR/Constants.h"         // For ConstantData, for instance.
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"  // For dbgs()
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfoMetadata.h" // For DILocation
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Metadata.h"

#include "Annotator.h"

#define DEBUG_TYPE "Annotator"

bool Annotator::runOnFunction(Function &F) {
  errs() << "I saw a function called " << F.getName() << "!\n";
  
  for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I){
    const DebugLoc d = I->getDebugLoc();

    if (d){
      unsigned line = d.getLine();
      // errs() << *I << " " << line << "\n";
    }
    else {
      errs() << *I << " no debug info\n";
    }
  }

  return true;

  LLVMContext& C = F.getContext();
  int instructions = 123;
  std::string s = "cesinha";
  if (!F.isDeclaration()) {
    for (auto I = inst_begin(F), E = inst_end(F); I != E; ++I) {
      // instructions++;
      // MDNode* N = MDNode::get(C, MDString::get(C, std::to_string(instructions)));
      MDNode* N = MDNode::get(C, MDString::get(C, s));
      (*I).setMetadata("stats.instNumber", N);
    }
    // MDNode* temp_N = MDNode::get(C, ConstantAsMetadata::get(ConstantInt::get(C, llvm::APInt(64, instructions, false))));
    // MDNode* N = MDNode::get(C, temp_N);
    // F.setMetadata("stats.totalInsts", N);
  }

  

  return true;
}

char Annotator::ID = 0;
static RegisterPass<Annotator> X("annotator",
    "Annotator");
