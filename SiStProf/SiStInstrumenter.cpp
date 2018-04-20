#include "llvm/Pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/Statistic.h"       // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"     // To use the iterator instructions
#include "llvm/IR/Instructions.h"     // To have access to the Instruction cls.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/Support/raw_ostream.h" // For dbgs()


#include "StoreTagger.h"
#include "SiStInstrumenter.h"

#define DEBUG_TYPE "SiStInstrumenter"


STATISTIC(InstSts, "Number of stores that have been instrumented.");
STATISTIC(NonISts, "Number of stores that have _not_ been instrumented.");


void SiStInstrumenter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<StoreTagger>();
  AU.setPreservesAll();
}


bool SiStInstrumenter::is_instrumentable(StoreInst *store) const {
  Value *store_vl = store->getValueOperand();
  Type *tp = store_vl->getType();
  return !tp->isVectorTy() && !tp->isStructTy() && !tp->isX86_MMXTy();
}


bool SiStInstrumenter::runOnFunction(Function &f) {
  errs() << "Function " << f.getName() << "\n";
  st_tagger = &getAnalysis<StoreTagger>();
  bool has_changed = false;
  for (Instruction &i : instructions(f)) {
    if(StoreInst *store = dyn_cast<StoreInst>(&i)) {
      if (is_instrumentable(store)) {
        this->insert_instrumentation(f, store);
        InstSts++;
        has_changed = true;
      } else {
        NonISts++;
      }
    }
  }
  DEBUG(dbgs() << "\nStores in " << f.getName() << ":\n" ; st_tagger->dump());
  return has_changed;
}


void SiStInstrumenter::insert_instrumentation(
    Function &f,
    StoreInst *store
) {
  // Create the load instruction that shall read the value in the target
  // address of the store instruction. We shall pass this value to the profiler.
  Value *comp_res = this->insert_store_query(f, store);
  // Create the profiling instructions.
  this->insert_logging_function(f, comp_res, store);
  // Save the store, so that we can output statistics about it later.
  worklist.insert(store);
}


Value* SiStInstrumenter::insert_store_query(
    Function &f,
    StoreInst* store
) {
  DEBUG(dbgs() << "Store is " << *store << '\n');
  LLVMContext &context = f.getContext();
  IRBuilder<> builder(store);
  // Create the load
  Value *target_address = store->getPointerOperand();
  Value *load = builder.CreateLoad(target_address, "prof_load");
  // Create the comparison
  Value *store_vl = store->getValueOperand();
  DEBUG(dbgs() << "Store's value type is " << *store_vl->getType() << '\n');
  DEBUG(dbgs() << "array type? " << store_vl->getType()->isArrayTy() << '\n');
  DEBUG(dbgs() << "struct? " << store_vl->getType()->isStructTy() << '\n');
  DEBUG(dbgs() << "XFP80? " << store_vl->getType()->isX86_FP80Ty() << '\n');
  DEBUG(dbgs() << "Vector? " << store_vl->getType()->isVectorTy() << '\n');
  Type *tp = store_vl->getType();
  Value *cmp;
  if (tp->isFloatingPointTy()) {
    cmp = builder.CreateFCmpOEQ(store_vl, load, "prof_cmp");
  } else {
    cmp = builder.CreateICmpEQ(store_vl, load, "prof_cmp");
  }
  // Convert the result of the comparison to an integer
  DEBUG(dbgs() << "Cmp is " << *cmp << '\n');
  DEBUG(dbgs() << "Type is " << *cmp->getType() << '\n');
  DEBUG(dbgs() << "Is vector? " << cmp->getType()->isVectorTy() << '\n');
  Value *iconv = builder.CreateZExt
    (cmp, FunctionType::getInt32Ty(context), "prof_ext");
  return iconv;
}


void SiStInstrumenter::insert_logging_function(
    Function &f,
    Value* comp_result,
    StoreInst* store
) {
  LLVMContext &context = f.getContext();
  Module *module = f.getParent();
  // Get a pointer to the function object that we are about to insert.
  const std::string function_name = "collect_data";
  Constant *const_fun = module->getOrInsertFunction(
      function_name,
      FunctionType::getVoidTy(context),
      Type::getInt32Ty(context), // Store identifier
      Type::getInt32Ty(context), // boolean (0 = is silent; 1 = is not silent)
      Type::getInt32Ty(context), // the type of the instruction)
      NULL);
  Function *new_function = cast<Function>(const_fun);
  // Fill up the arguments of the function into the vector args
  std::vector<Value*> args;
  // * First argument: the store identifier:
  int id_aux = st_tagger->get_id(store);
  Constant *store_id_llvm = ConstantInt::get(context, APInt(32, id_aux, false));
  args.push_back(store_id_llvm);
  // * Second argument: the boolean that says if the store is silent or not:
  args.push_back(comp_result);
  // * Third arg: the type of the stored data.
  int tp_aux = store->getType()->getTypeID();
  Constant *store_tp_llvm = ConstantInt::get(context, APInt(32, tp_aux, false));
  args.push_back(store_tp_llvm);
  // Insert the function in the code
  IRBuilder<> builder(store);
  builder.CreateCall(new_function, args);
}


void SiStInstrumenter::dump() const {
  for (auto store : this->worklist) {
    errs() << *store << '\n';
  }
}


char SiStInstrumenter::ID = 0;
static RegisterPass<SiStInstrumenter> X("ssProf", "Silent Store Profiler");
