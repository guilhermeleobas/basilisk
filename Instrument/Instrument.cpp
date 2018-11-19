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

std::map<std::string, Value*> variables;
std::map<std::string, Value*> count_variables;


Value* Instrument::get_or_create_string(Instruction *I){

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
  Constant *const_function = M.getOrInsertFunction("dump_csv",
    FunctionType::getVoidTy(M.getContext()));

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
    Type::getInt8PtrTy(M.getContext()));

  Function *f = cast<Function>(const_function);

  // Create the parameter for the call
  Value *v = get_or_create_string(I);
  
  // Fill the parameter
  std::vector<Value *> args;
  args.push_back(v);

  // Create the call
  Builder.CreateCall(f, args);
}

int Instrument::getNumPredecessors(BasicBlock *BB){
  int cnt = 0;
  
  for (BasicBlock *Pred : predecessors(BB))
    cnt++;

  return cnt;
}


bool Instrument::runOnModule(Module &M) {
  for (auto &F : M){
    for (auto &BB : F){
      for (auto &I : BB){
        if (CallInst *call = dyn_cast<CallInst>(&I)){
          insert_call(M, call);
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

  for (auto &F : M){
    for (auto &BB : F){
      for (auto &I : BB){
        
        switch(I.getOpcode()){
          // Memory Access
          case Instruction::Store:
          case Instruction::Load:
          
          // Binary operators
          case Instruction::Add:
          case Instruction::FAdd:
          case Instruction::Sub:
          case Instruction::FSub:
          case Instruction::Mul:
          case Instruction::FMul:
          case Instruction::UDiv:
          case Instruction::SDiv:
          case Instruction::FDiv:
          case Instruction::URem:
          case Instruction::SRem:
          case Instruction::FRem:
            
          // Logical Operators
          case Instruction::And:
          case Instruction::Or:
          case Instruction::Xor:
          
          // Other instructions
          case Instruction::ICmp:
          case Instruction::FCmp:
          case Instruction::Call:
          case Instruction::Select:
          case Instruction::Shl:
          case Instruction::LShr:
          case Instruction::AShr:
            insert_call(M, &I);
        }
      }
    }
  }

  for (auto &F : M){
    for (auto &BB : F){
      if (getNumPredecessors(&BB) >= 2){
        Instruction *ins = BB.getTerminator();
        insert_call(M, ins);
      }
    }
  }

  return true;
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("Instrument",
    "Instrument");
