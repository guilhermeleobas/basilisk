#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"        // For ConstantInt, ConstantFP, etc.
#include "llvm/ADT/Statistic.h"       // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"     // To iterate over instructions.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/IR/Instructions.h"     // To have access to the Instruction cls.
#include "llvm/Support/raw_ostream.h" // For dbgs()

#include "ValOriginAnalysis.h"

#define BUF_SIZE (4 * VORI_MAX_MODI + 1)


/**
 * This method converts a modifier into a string. To avoid to have to free the
 * string all the time, we allocate it statically, and always returns a
 * pointer to this buffer.
 * @param vor: the word that shall be transformed into a string.
 * @return a pointer to a static buffer, containing the string version of vor.
 */
const char* to_str(ValOrigin vor) {
  static char buf[BUF_SIZE];
  char *aux = buf;
  for (int i = 0; i < VORI_MAX_MODI; i++) {
    aux[0] = ' ';
    aux[1] = ' ';
    aux[2] = (vor == i) ? '1': '0';
    aux[3] = ',';
    aux += 4;
  }
  buf[BUF_SIZE - 1] = '\0';
  return buf;
}


/**
 * This method helps in debugging. It simply prints a list of columns, in the
 * same order as the method 'to_str' transforms a word into a string.
 * @return a constant string with the column names.
 */
const char* get_val_origin_names() {
  return "OZR,OIN,OFP,OGB,OAY,OAG,OLD,OCL,OMX,OAD,OIC,ONG,ONT,OMD,"
    "OMU,OBN,OCS,OAL,OUN,OIY";
}


#define DEBUG_TYPE "ValOriginAnalysis"


void ValOriginAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


bool ValOriginAnalysis::runOnFunction(Function &f) {
  compute_origins(f);
  return false;
}


void ValOriginAnalysis::compute_origins(const Function &f) {
  DEBUG(dbgs() << "Analyzing " << f.getName() << '\n');
  for (const Instruction &i : instructions(f)) {
    if(const StoreInst *store = dyn_cast<StoreInst>(&i)) {
      this->worklist[store] = this->val2origin(store->getValueOperand());
    }
  }
  DEBUG(dump());
}


void ValOriginAnalysis::dump() const {
  errs() << "Value origin analysis:\n";
  // Print all the instructions first:
  for (auto pair : this->worklist) {
    errs() << *pair.first << '\n';
  }
  // Now, print their abstract state:
  errs() << get_val_origin_names() << '\n';
  for (auto pair : this->worklist) {
    errs() << to_str(pair.second) << '\n';
  }
}


bool is_inc(const BinaryOperator *binop) {
  const Constant *const_0 = dyn_cast<Constant>(binop->getOperand(0));
  if (const_0 && const_0->isOneValue()) {
    return true;
  }
  const Constant *const_1 = dyn_cast<Constant>(binop->getOperand(1));
  if (const_1 && const_1->isOneValue()) {
    return true;
  }
  return false;
}


bool is_mad(const BinaryOperator *binop) {
  const Instruction *inst_0 = dyn_cast<Instruction>(binop->getOperand(0));
  if (inst_0 &&
      (inst_0->getOpcode() == Instruction::Mul ||
       inst_0->getOpcode() == Instruction::FMul)
  ) {
    return true;
  }
  const Instruction *inst_1 = dyn_cast<Instruction>(binop->getOperand(1));
  if (inst_1 &&
      (inst_1->getOpcode() == Instruction::Mul ||
       inst_1->getOpcode() == Instruction::FMul)
  ) {
    return true;
  }
  return false;
}


ValOrigin ValOriginAnalysis::val2origin(const Value* v) const {
  if (isa<Argument>(v)) {
    return VORI_ARGUMENT;
  } else if (const Constant *cnt = dyn_cast<Constant>(v)) {
    if (cnt->isZeroValue()) {
      return VORI_CONS_ZER;
    } else if (isa<ConstantInt>(v)) {
      return VORI_CONS_INT;
    } else if (isa<ConstantFP>(v)) {
      return VORI_CONS_FLP;
    } else if (isa<GlobalVariable>(v)) {
      // TODO: maybe we can remove this entry, because if a variable is stored
      // from a global variable, it will be loaded first. See LoadInst below.
      return VORI_CONS_GLB;
    } else {
      return VORI_CONS_ANY;
    }
  } else if (isa<Instruction>(v)) {
    if (const LoadInst *load = dyn_cast<LoadInst>(v)) {
      if (isa<GlobalVariable>(load->getPointerOperand())) {
        return VORI_CONS_GLB;
      } else {
        return VORI_INST_LOA;
      }
    } else if (isa<CallInst>(v) || isa<InvokeInst>(v)) {
      return VORI_INST_CAL;
    } else if (isa<PHINode>(v) || isa<SelectInst>(v)) {
      return VORI_INST_MUX;
    } else if (const BinaryOperator *binop = dyn_cast<BinaryOperator>(v)) {
      if (BinaryOperator::isNeg(binop) || BinaryOperator::isFNeg(binop)) {
        return VORI_INST_NEG;
      } else if (BinaryOperator::isNot(binop)) {
        return VORI_INST_NOT;
      } else {
        switch(binop->getOpcode()) {
          case Instruction::Add:
          case Instruction::FAdd:
          case Instruction::Sub:
          case Instruction::FSub:
            if (is_inc(binop)) {
              return VORI_INST_INC;
            } else if (is_mad(binop)) {
              return VORI_INST_MAD;
            } else {
              return VORI_INST_ADD;
            }
          case Instruction::Mul:
          case Instruction::FMul:
          case Instruction::UDiv:
          case Instruction::SDiv:
          case Instruction::FDiv:
          case Instruction::URem:
          case Instruction::SRem:
          case Instruction::FRem:
            return VORI_INST_MUL;
          default:
            return VORI_INST_BIN;
        }
      }
    } else if (const Instruction *unop = dyn_cast<UnaryInstruction>(v)) {
      switch(unop->getOpcode()) {
        case Instruction::BitCast:
        case Instruction::FPExt:
        case Instruction::FPToUI:
        case Instruction::FPToSI:
        case Instruction::UIToFP:
        case Instruction::SIToFP:
        case Instruction::IntToPtr:
        case Instruction::FPTrunc:
        case Instruction::Trunc:
        case Instruction::SExt:
        case Instruction::ZExt:
          return VORI_INST_CAS;
        case Instruction::Alloca:
          return VORI_INST_ALL;
        default:
          return VORI_INST_UNR;
      }
    } else {
      return VORI_INST_ANY;
    }
  }
  assert(false && "Value of unknown origin.");
}


char ValOriginAnalysis::ID = 0;
static RegisterPass<ValOriginAnalysis> X("valOrAnalysis", "Val-orig Analysis.");
