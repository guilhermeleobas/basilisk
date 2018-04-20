#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"        // For ConstantInt, ConstantFP, etc.
#include "llvm/ADT/Statistic.h"       // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"     // To iterate over instructions.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/Support/raw_ostream.h" // For dbgs()


#include "ModAnalysis.h"

#define BUF_SIZE 129

/**
 * This method converts a modifier into a string. To avoid to have to free the
 * string all the time, we allocate it statically, and always returns a
 * pointer to this buffer.
 * @param md: the word that shall be transformed into a string.
 * @return a pointer to a static buffer, containing the string version of md.
 */
const char* to_str(ModifierWord md) {
  static char buf[BUF_SIZE];
  char *aux = buf;
  for (int i = 0; i < MAX_MODIFIER; i++) {
    aux[0] = ' ';
    aux[1] = ' ';
    aux[2] = (md & (1 << i)) ? '1': '0';
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
const char* get_column_names() {
  return "ZER,INT,FTP,NUL,GLB,FUN,CN?,ARG, LD,CAL,CMP,PHI,SEL,ADD,SUB,MUL,DIV,"
         "REM,SHF,BIT,GEP,CAS,EXT,F2I,I2F,I2P,P2I,TRN,SEX,ZEX,ALL,IN?,";
}


/**
 * This method adds a new bit to a modifier. Notice that if this modifier is
 * out-of-range, then md is not changed.
 * @param md: the word that will be updated.
 * @param new_mod: the new modifier of that word.
 * @return a new word, formed by md U {new_mod}.
 */
ModifierWord set_modifier(const ModifierWord md, const Modifier new_mod) {
  assert(new_mod <= MAX_MODIFIER && "Invalid modifer!");
  return md | (1 << new_mod);
}


/**
 * Produces an empty word. This is the initial abstract state of all the
 * values in our lattice.
 * @return an empty word.
 */
ModifierWord get_initial_state() {
  return 0U;
}


/**
 * Performs the meed operation onto two modifier words. The meed operation is
 * the union. We implement union via a simple binary or operation.
 * @param m1: the first element in the meet.
 * @param m2: the second element in the meet.
 * @return m1 meet m2, e.g., the union of both the words.
 */
ModifierWord modifier_meet(const ModifierWord m1, const ModifierWord m2) {
  return m1 | m2;
}


#define DEBUG_TYPE "ModAnalysis"


void ModAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
}


bool ModAnalysis::runOnFunction(Function &f) {
  DEBUG(dbgs() << "Analyzing " << f.getName() << '\n');
  run_dataflow_analysis(f);
  DEBUG(dump());
  return false;
}


void ModAnalysis::run_dataflow_analysis(const Function &f) {
  bool has_changed;
  do {
    has_changed = false;
    for (const Instruction &i : instructions(f)) {
      // Perform a meet on the operands of the instruction:
      ModifierWord original_status = this->get_status(&i);
      ModifierWord new_status = original_status;
      for (const Value *v : i.operands()) {
        new_status = modifier_meet(new_status, this->get_status(v));
      }
      // Get the status of the instruction itself:
      new_status = set_modifier(new_status, get_modifier(i));
      if (new_status != original_status) {
        this->worklist[&i] = new_status;
        has_changed = true;
      }
    }
  } while (has_changed);
}


void ModAnalysis::dump() const {
  // Print all the instructions first:
  for (auto pair : this->worklist) {
    errs() << *pair.first << '\n';
  }
  // Now, print their abstract state:
  errs() << get_column_names() << '\n';
  for (auto pair : this->worklist) {
    errs() << to_str(pair.second) << '\n';
  }
}


Modifier ModAnalysis::get_modifier(const Instruction &i) const {
  switch(i.getOpcode()) {
    case Instruction::Load:
      return INST_LOAD;
    case Instruction::Call:
      return INST_CALL;
    case Instruction::ICmp:
    case Instruction::FCmp:
      return INST_COMP;
    case Instruction::PHI:
      return INST_PHI;
    case Instruction::Select:
      return INST_SELECT;
    case Instruction::Add:
    case Instruction::FAdd:
      return INST_ADD;
    case Instruction::Sub:
    case Instruction::FSub:
      return INST_SUB;
    case Instruction::Mul:
    case Instruction::FMul:
      return INST_MUL;
    case Instruction::UDiv:
    case Instruction::SDiv:
    case Instruction::FDiv:
      return INST_DIV;
    case Instruction::URem:
    case Instruction::SRem:
    case Instruction::FRem:
      return INST_REM;
    case Instruction::Shl:
    case Instruction::LShr:
    case Instruction::AShr:
      return INST_BITWISE_SHF;
    case Instruction::And:
    case Instruction::Or:
    case Instruction::Xor:
      return INST_BITWISE_LOGICAL;
    case Instruction::GetElementPtr:
      return INST_GEP;
    case Instruction::BitCast:
      return INST_BIT_CAST;
    case Instruction::FPExt:
      return INST_FP_EXT;
    case Instruction::FPToUI:
    case Instruction::FPToSI:
      return INST_FP_TO_INT;
    case Instruction::UIToFP:
    case Instruction::SIToFP:
      return INST_INT_TO_FP;
    case Instruction::IntToPtr:
      return INST_INT_TO_PTR;
    case Instruction::PtrToInt:
      return INST_PTR_TO_INT;
    case Instruction::FPTrunc:
    case Instruction::Trunc:
      return INST_TRUNC;
    case Instruction::SExt:
      return INST_SIGN_EXT;
    case Instruction::ZExt:
      return INST_ZERO_EXT;
    case Instruction::Alloca:
      return INST_ALLOCA;
    default:
      return INST_UNKNOWN;
  }
}


ModifierWord ModAnalysis::get_status(const Value* v) const {
  if (isa<Argument>(v)) {
    return set_modifier(get_initial_state(), ARGUMENT);
  } else if (const Constant *cnt = dyn_cast<Constant>(v)) {
    if (cnt->isZeroValue()) {
      return set_modifier(get_initial_state(), CONST_ZERO);
    } else if (isa<ConstantInt>(v)) {
      return set_modifier(get_initial_state(), CONST_INT);
    } else if (isa<ConstantFP>(v)) {
      return set_modifier(get_initial_state(), CONST_FP);
    } else if (isa<ConstantPointerNull>(v)) {
      return set_modifier(get_initial_state(), CONST_NULL);
    } else if (isa<Function>(v)) {
      return set_modifier(get_initial_state(), CONST_FUNCTION);
    } else if (isa<GlobalVariable>(v)) {
      return set_modifier(get_initial_state(), CONST_GLOBAL);
    } else {
      return set_modifier(get_initial_state(), CONST_UNKNOWN);
    }
  } else {
    const Instruction *inst = dyn_cast<Instruction>(v);
    auto status = this->worklist.find(inst);
    if (status != this->worklist.end()) {
      return status->second;
    } else {
      return get_initial_state();
    }
  }
}


char ModAnalysis::ID = 0;
static RegisterPass<ModAnalysis> X("modAnalysis", "Modifier Analysis.");
