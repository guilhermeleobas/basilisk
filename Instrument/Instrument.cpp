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

#define DEBUG_TYPE "Instrument"
#define COUNTER "DCC888_counter"

void Instrument::print_instructions(Module &M){
  for (auto &F : M){
    for (auto &BB : F){
      for (auto &I : BB){
        errs() << I << "\n";
      }
    }
  }
}

const std::string Instrument::get_function_name (Instruction *I){
  // return "count_instruction";


  switch (I->getOpcode()){
    case Instruction::Store:
      return "increment_store_count";
    case Instruction::Load:
      return "increment_load_count";
    case Instruction::Add:
      return "increment_add_count";
    case Instruction::Mul:
      return "increment_mul_count";
    default:
      return "dump";
  }
}

Value* Instrument::Alloc_string_space(Module &M, const std::string str, Instruction *I){
  IRBuilder<> Builder(I);

  // Create Alloca Instruction
  unsigned size = 10;
  auto arrayType = llvm::ArrayType::get(llvm::IntegerType::get(M.getContext(), 8), size);
  AllocaInst *Alloca = Builder.CreateAlloca(arrayType);

  // Create Store Instruction
  auto *s = ConstantDataArray::getString(M.getContext(), StringRef(str));
  return Builder.CreateStore(s, Alloca);
}


Function* Instrument::build_call(Module &M, const std::string function_name){
  Constant *const_function = M.getOrInsertFunction(function_name,
    FunctionType::getVoidTy(M.getContext()),
    // Type::getInt8PtrTy(M.getContext()),
    NULL);

  const std::string str = "store";
  std::vector<Value*> args;
  // ConstantArray *ca = ConstantArray::get(M.getContext(), str.c_str());
  // args.push_back(ca);

  return cast<Function>(const_function);
}


bool Instrument::runOnModule(Module &M) {


  for (auto &F : M){
    errs() << "Function: " << F.getName() << "\n";
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
        else if (ReturnInst *ri = dyn_cast<ReturnInst>(&I)){
          if (F.getName() == "main")
            insert_call(M, ri);
        }

      }
    }
  }

  return true;
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("Instrument",
    "Instrument");
