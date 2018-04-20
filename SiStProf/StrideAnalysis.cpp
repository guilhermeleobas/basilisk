#include "llvm/Pass.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/IR/InstIterator.h"     // To iterate over instructions.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/Support/raw_ostream.h" // For dbgs()
#include "llvm/Support/raw_ostream.h" // For dbgs()
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopInfo.h"

#include "Strides.h"

#include "StrideAnalysis.h"


/**
 * This method converts a StrideProp into a string. To avoid to have to free
 * the string all the time, we allocate it statically, and always returns a
 * pointer to this buffer.
 * @param stride_w: the word that shall be transformed into a string.
 * @return a pointer to a static buffer, containing the string version of vor.
 */
const char* strideProp2str(StridePropWord w) {
  int const BUF_SIZE = SCEV_MAX * 4 + 1;
  static char buf[BUF_SIZE];
  char *aux = buf;
  for (int i = 0; i < SCEV_MAX; i++) {
    aux[0] = ' ';
    aux[1] = ' ';
    aux[2] = (w & (1 << i)) ? '1': '0';
    aux[3] = ',';
    aux += 4;
  }
  buf[BUF_SIZE - 1] = '\0';
  return buf;
}


/**
 * This method helps in debugging. It simply prints a list of columns, in the
 * same order as the method 'strideProp2str' transforms a word into a string.
 * @return a constant string with the column names.
 */
const char* get_stride_feature_names() {
  return "Erc,Eaf,Eqd,Es1,Es4,Es8,EsN,Esp,Etp";
}


/**
 * This method adds a new bit to a StrideProp. Notice that if this
 * property is out-of-range, then w is not changed.
 * @param w: the word that will be updated.
 * @param new_w: the new property of that word.
 * @return a new word, formed by w U {new_w}.
 */
StridePropWord set_stride_prop(
    const StridePropWord w,
    const StrideProp new_w
) {
  assert(new_w <= SCEV_MAX && "Invalid stride property!");
  return w | (1 << new_w);
}


/**
 * Produces an empty word. This is the initial abstract state of all the
 * values in our lattice.
 * @return an empty word.
 */
StridePropWord get_initial_stride_prop() { return 0U; }

#define DEBUG_TYPE "strides"

void StrideAnalysis::getAnalysisUsage(AnalysisUsage &analyses) const {
  analyses.addRequired<ScalarEvolutionWrapperPass>();
  analyses.addRequired<LoopInfoWrapperPass>();
  analyses.setPreservesAll();
}


bool StrideAnalysis::runOnFunction(Function &f) {
  DEBUG(dbgs() << "Analyzing " << f.getName() << '\n');
  LoopInfo &lp_inf = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  ScalarEvolution &scalar_ev = 
    getAnalysis<ScalarEvolutionWrapperPass>().getSE();
  this->collect_properties(f, lp_inf, scalar_ev);
  DEBUG(dump());
  return false;
}


void StrideAnalysis::collect_properties(
    Function &f,
    LoopInfo &lp_inf,
    ScalarEvolution &scalar_ev
) {
  // If you need to print the results of scalar evolution, do:
  // scalar_ev.print(errs());
  for (Instruction &i : instructions(f)) {
    if(StoreInst *store = dyn_cast<StoreInst>(&i)) {
      StridePropWord w = get_initial_stride_prop();
      Value *ptr = store->getPointerOperand();
      if (scalar_ev.isSCEVable(ptr->getType())) {
        const SCEV* scev = scalar_ev.getSCEV(ptr);
        DEBUG(dbgs() << "Store = " << *store << '\n');
        DEBUG(dbgs() << "- Pointer = " << *ptr << '\n');
        DEBUG(dbgs() << " - SCEV = " << *scev << '\n');
        if (const SCEVAddRecExpr *rec_addr = dyn_cast<SCEVAddRecExpr>(scev)) {
          w = set_stride_prop(w, SCEV_REC);
          DEBUG(dbgs() << " - Recurrence address = " << *rec_addr << '\n');
          if (rec_addr->isAffine()) {
            w = set_stride_prop(w, SCEV_AFF);
          }
          DEBUG(dbgs() << " - Affine? " << rec_addr->isAffine() << '\n');
          if (rec_addr->isQuadratic()) {
            w = set_stride_prop(w, SCEV_AFF);
          }
          DEBUG(dbgs() << " - Quadratic? " << rec_addr->isQuadratic() << '\n');
          const SCEV* step = rec_addr->getStepRecurrence(scalar_ev);
          if (const SCEVConstant *trip = dyn_cast<SCEVConstant>(step)) {
            const llvm::APInt trip_x = trip->getValue()->getValue();
            int trip_u = trip_x.getSExtValue();
            if (trip_u == 1) {
              w = set_stride_prop(w, SCEV_ST1);
            } else if (trip_u == 4) {
              w = set_stride_prop(w, SCEV_ST4);
            } else if (trip_u == 8) {
              w = set_stride_prop(w, SCEV_ST8);
            } else {
              w = set_stride_prop(w, SCEV_STN);
            }
            DEBUG(dbgs() << " - St = " << trip_u << '\n');
            const PointerType *ptp =
              dyn_cast<PointerType>(ptr->getType());
            int type_size = ptp->getElementType()->getPrimitiveSizeInBits();
            DEBUG(dbgs() << " - Tp = " << type_size << '\n');
            if (type_size && type_size == 8 * trip_u) {
              w = set_stride_prop(w, SCEV_SPT);
            }
          }
        }
      }
      Loop* inner_loop = lp_inf.getLoopFor(store->getParent());
      if (inner_loop) {
        if (scalar_ev.hasLoopInvariantBackedgeTakenCount(inner_loop)) {
          unsigned trip_count =
            scalar_ev.getSmallConstantTripCount(inner_loop);
          DEBUG(dbgs() << " - TC = " << trip_count << '\n');
          w = set_stride_prop(w, SCEV_TPC);
        }
      }
      this->worklist[store] = w;
    }
  }
}

void StrideAnalysis::dump() const {
  // Print all the instructions first:
  for (auto pair : this->worklist) {
    errs() << *pair.first << '\n';
  }
  // Now, print their abstract state:
  errs() << get_stride_feature_names() << '\n';
  for (auto pair : this->worklist) {
    errs() << strideProp2str(pair.second) << '\n';
  }
}

char StrideAnalysis::ID = 0;
static RegisterPass<StrideAnalysis> X("strides", "Stride Analysis.");
