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

GlobalVariable* Instrument::create_global(Module &M){
  IRBuilder<> Builder(M.getContext());
  M.getOrInsertGlobal(COUNTER, Builder.getInt32Ty());

  GlobalVariable *gv = M.getNamedGlobal(COUNTER);

  gv->setLinkage(GlobalValue::CommonLinkage);
  gv->setAlignment(4);

  ConstantInt* const_int_val = ConstantInt::get(M.getContext(), APInt(32,0));
  gv->setInitializer(const_int_val);

  return gv;
}

Value* Instrument::increment(Module& M, GlobalVariable* gv, Instruction* I){

  IRBuilder<> Builder(I);
  
  // Create the load
  Value *target_address = gv;
  Value *load = Builder.CreateLoad(target_address, "counter_ptr");
  
  // Create the add inst
  Value* one = ConstantInt::get(Type::getInt32Ty(M.getContext()),1);
  Value* addInst = Builder.CreateAdd(load, one);
  Value* store = Builder.CreateStore(addInst, target_address);
  
  return store;
}

void Instrument::insert_call(Module &M, GlobalValue *gv, Instruction *I){
  IRBuilder<> Builder(I);

  // Let's create a call to the function collect_d
  // for some reason, OPT breaks if I use the name 'collect_data'
  const std::string function_name = "collect_d";
  Constant *const_function = M.getOrInsertFunction(function_name, 
    FunctionType::getVoidTy(M.getContext()),
    Type::getInt32Ty(M.getContext()), // Store identifier
    NULL);
  Function *collect_function = cast<Function>(const_function);

  // Fill up the arguments of the function into the vector args
  // Since gv is a pointer to a global value, we need to get
  // the address first
  Value *load = Builder.CreateLoad(gv, "counter_ptr");
  std::vector<Value*> args;
  args.push_back(load);

  // Create the call
  Builder.CreateCall(collect_function, args);
}

bool Instrument::runOnModule(Module &M) {

  GlobalVariable* gv = create_global(M);

  for (auto &F : M){
    errs() << "Function: " << F.getName() << "\n";
    for (auto &BB : F){
      for (auto &I : BB){
        // errs() << I << "\n";
        
        if (ReturnInst *ri = dyn_cast<ReturnInst>(&I)){
          errs() << "Return inst: " << *ri << "\n";
          if (F.getName() == "main")
            insert_call(M, gv, ri);
        }

        // Let's count the number of stores
        if (StoreInst *store = dyn_cast<StoreInst>(&I)){
          increment(M, gv, store);
        }

      }
    }
  }

  return true;
}

char Instrument::ID = 0;
static RegisterPass<Instrument> X("Instrument",
    "Instrument");
