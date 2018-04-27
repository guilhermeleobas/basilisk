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
#include "llvm/IR/IRBuilder.h"

#include "Instrument.h"

#include <map>

#define DEBUG_TYPE "Instrument"
#define COUNTER "DCC888_counter"

std::map<std::string, Value*> variables;

void Instrument::print_instructions(Module &M){
  for (auto &F : M){
    for (auto &BB : F){
      for (auto &I : BB){
        errs() << I << "\n";
      }
    }
  }
}


Value* Instrument::alloc_string(Instruction *I){

  /* 
  A variable is just a char* with some text identifying the instruction.
  For instance, we use the getOpcodeName() function.
  */
  const std::string opcodeName = I->getOpcodeName();

  // If we already create a variable for the opcodeName, let's just return it
  if (variables.find(opcodeName) != variables.end())
    return variables[opcodeName];

  // Otherwise, let's create one
  IRBuilder<> Builder(I);
  Value *var = Builder.CreateGlobalStringPtr(StringRef(opcodeName));
  variables[opcodeName] = var;

  return var;
}

void Instrument::insert_dump_call(Module &M, Instruction *I){
  IRBuilder<> Builder(I);

  // Let's create the function call
  Constant *const_function = M.getOrInsertFunction("dump",
    FunctionType::getVoidTy(M.getContext()),
    NULL);

  Function *f = cast<Function>(const_function);

  // Create the call
  Builder.CreateCall(f, std::vector<Value*>());
}


void Instrument::insert_call(Module &M, Instruction *I){
  IRBuilder<> Builder(I);

  // Let's create the function call
  const std::string function_name = "count_instruction";

  Constant *const_function = M.getOrInsertFunction(function_name,
    FunctionType::getVoidTy(M.getContext()),
    Type::getInt8PtrTy(M.getContext()),
    NULL);

  Function *f = cast<Function>(const_function);

  // Let's create the parameter for the call
  Value *v = alloc_string(I);
  
  // Fill the parameter
  std::vector<Value *> args;
  args.push_back(v);

  // Create the call
  Builder.CreateCall(f, args);
}


bool Instrument::runOnModule(Module &M) {

  for (auto &F : M){
    for (auto &BB : F){
      for (auto &I : BB){
        
        if (StoreInst *store = dyn_cast<StoreInst>(&I)){
          insert_call(M, store);
        }
        else if (LoadInst *load = dyn_cast<LoadInst>(&I)){
          insert_call(M, load);
        }
        else if (BinaryOperator *bin = dyn_cast<BinaryOperator>(&I)){
          insert_call(M, bin);
        }
        else if (ICmpInst *icmp = dyn_cast<ICmpInst>(&I)){
          insert_call(M, icmp);
        }
        else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(&I)){
          insert_call(M, fcmp);
        }
        else if (BranchInst *br = dyn_cast<BranchInst>(&I)){
          insert_call(M, br);
        }
        else if (IndirectBrInst *bri = dyn_cast<IndirectBrInst>(&I)){
          insert_call(M, bri);
        }
        else if (ReturnInst *ri = dyn_cast<ReturnInst>(&I)){
          if (F.getName() == "main")
            insert_dump_call(M, ri);
        }
        else if (CallInst *ci = dyn_cast<CallInst>(&I)){
          Function *fun = ci->getCalledFunction();
          if (fun){
            std::string name = fun->getName();
            if (name == "exit")
              insert_dump_call(M, ci);
          }
        }

      }
    }
  }

  return true;
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("Instrument",
    "Instrument");
