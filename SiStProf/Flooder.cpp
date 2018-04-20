#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"     // To iterate over instructions.
#include "llvm/IR/Instructions.h"     // To have access to CallInst.


#include <numeric>
#include <functional>

#include "Flooder.h"

/*
 * Notice that we treat phi-functions differently. For a phi-function to be
 * interesting, it is enough that ANY of its arguments is interesting. In
 * other words, it is like if all its arguments were in an UNDEF state, where
 * UNDEF meet ANY is ANY.
 */
void Flooder::compute_interesting_values() {
  bool has_changed;
  do {
    has_changed = false;
    for (const llvm::Instruction &i : instructions(this->function)) {
      if(!this->is_interesting(&i)) {
        bool is_potentially_interesting;
        switch (i.getOpcode()) {
          case llvm::Instruction::PHI: {
            is_potentially_interesting =
              std::accumulate(i.operands().begin(), i.operands().end(), false,
                [this](bool a, const llvm::Value *b) {
                  return a || this->is_interesting(b);
                });
            break;
          }
          case llvm::Instruction::Select: {
            const llvm::SelectInst *sel = llvm::dyn_cast<llvm::SelectInst>(&i);
            is_potentially_interesting =
              this->is_interesting(sel->getTrueValue()) &&
              this->is_interesting(sel->getFalseValue());
            break;
          }
          default: {
            is_potentially_interesting =
              std::accumulate(i.operands().begin(), i.operands().end(), true,
                [this](bool a, const llvm::Value *b) {
                  return a && this->is_interesting(b);
                });
          }
        }
        if (is_potentially_interesting) {
          has_changed = true;
          this->add_interesting(&i);
        }
      }
    }
  } while (has_changed);
}
