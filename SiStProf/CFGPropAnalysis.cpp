#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"        // For ConstantInt, ConstantFP, etc.
#include "llvm/ADT/Statistic.h"       // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"     // To iterate over instructions.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/IR/Instructions.h"     // To have access to the Instruction cls.

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h" // For dbgs()
#include "llvm/Analysis/PostDominators.h"

#include "CFGPropAnalysis.h"

#define BUF_SIZE (4 * STPR_MAX + 1)

/**
 * This method converts a CFGProperty into a string. To avoid to have to free
 * the string all the time, we allocate it statically, and always returns a
 * pointer to this buffer.
 * @param cp: the word that shall be transformed into a string.
 * @return a pointer to a static buffer, containing the string version of vor.
 */
const char* cfgProp2str(CFGPropertyWord cp) {
  static char buf[BUF_SIZE];
  char *aux = buf;
  for (int i = 0; i < STPR_MAX; i++) {
    aux[0] = ' ';
    aux[1] = ' ';
    aux[2] = (cp & (1 << i)) ? '1': '0';
    aux[3] = ',';
    aux += 4;
  }
  buf[BUF_SIZE - 1] = '\0';
  return buf;
}


/**
 * This method helps in debugging. It simply prints a list of columns, in the
 * same order as the method 'cfgProp2str' transforms a word into a string.
 * @return a constant string with the column names.
 */
const char* get_cfg_feature_names() {
  return "Smn,Sl0,Sl1,Sl2,Sl3,Sln,Scm,Scl,Spl,Slx,Smp,Sms,Ssl,Sdl,Svi,Spi";
}


/**
 * This method adds a new bit to a CFGProperty. Notice that if this property is
 * out-of-range, then cp is not changed.
 * @param cp: the word that will be updated.
 * @param new_cp: the new property of that word.
 * @return a new word, formed by cp U {new_cp}.
 */
CFGPropertyWord set_prop(const CFGPropertyWord cp, const CFGProperty new_cp) {
  assert(new_cp <= STPR_MAX && "Invalid cfg property!");
  return cp | (1 << new_cp);
}


/**
 * Produces an empty word. This is the initial abstract state of all the
 * values in our lattice.
 * @return an empty word.
 */
CFGPropertyWord get_initial_cfg_prop() { return 0U; }


#define DEBUG_TYPE "CFGPropAnalysis"


void CFGPropAnalysis::getAnalysisUsage(AnalysisUsage &analyses) const {
  analyses.addRequired<LoopInfoWrapperPass>();
  analyses.addRequired<DominatorTreeWrapperPass>() ;
  analyses.addRequired<PostDominatorTree>();
  analyses.setPreservesAll();
}


bool CFGPropAnalysis::runOnFunction(Function &f) {
  DEBUG(dbgs() << "Analyzing " << f.getName() << '\n');
  const LoopInfo &lp_inf =
    getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  const PostDominatorTree& pd_tree = getAnalysis<PostDominatorTree>();
  DominatorTree& dm_tree = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  collect_properties(f, &lp_inf, &dm_tree, &pd_tree);
  DEBUG(dump());
  return false;
}


int num_predecessors(const BasicBlock* bb) {
  int sum = 0;
  for (auto it = pred_begin(bb), et = pred_end(bb); it != et; ++it) {
    sum++;
  }
  return sum;
}


void CFGPropAnalysis::collect_properties(
    const Function &f,
    const LoopInfo *loop_info,
    const DominatorTree *dm_tree,
    const PostDominatorTree *pd_tree
) {
  // FIXME: should we clear the worklist before running the algorithm? If we
  // don't do it, then there will be lots of data being kept from the analysis
  // of one function to the other.
  for (const Instruction &i : instructions(f)) {
    if(const StoreInst *store = dyn_cast<StoreInst>(&i)) {
      CFGPropertyWord cp = get_initial_cfg_prop();
      const BasicBlock *parent_block = store->getParent();
      // Check if the store is within the main function:
      if (f.getName() == "main") {
        cp = set_prop(cp, STPR_MAINFN);
      }
      // Check if the store is always executed:
      const BasicBlock &entry = f.getEntryBlock();
      bool is_compulsory = pd_tree->dominates(parent_block, &entry);
      if (is_compulsory) {
        cp = set_prop(cp, STPR_CMPSRY);
      }
      // Get the number of loops that exist around the store:
      switch (loop_info->getLoopDepth(parent_block)) {
        case 0: cp = set_prop(cp, STPR_LOOP_0);
                break;
        case 1: cp = set_prop(cp, STPR_LOOP_1);
                break;
        case 2: cp = set_prop(cp, STPR_LOOP_2);
                break;
        case 3: cp = set_prop(cp, STPR_LOOP_3);
                break;
        default: cp = set_prop(cp, STPR_LOOP_n);
      }
      const Loop* inner_loop = loop_info->getLoopFor(parent_block);
      if (inner_loop) {
        // Checks if the store is loop invariant:
        if (inner_loop->isLoopInvariant(store->getValueOperand())) {
          cp = set_prop(cp, STPR_VL_INV);
        }
        if (inner_loop->isLoopInvariant(store->getPointerOperand())) {
          cp = set_prop(cp, STPR_PT_INV);
        }
        // Count the number of exits in the loop:
        if (inner_loop->hasDedicatedExits() &&
            inner_loop->getUniqueExitBlock()
        ) {
          cp = set_prop(cp, STPR_LPEXUN);
        }
        // Check if the store is compulsory within the loop:
        const BasicBlock *loop_entry = inner_loop->getHeader();
        bool is_comp_in_loop = pd_tree->dominates(parent_block, loop_entry);
        if (is_comp_in_loop) {
          cp = set_prop(cp, STPR_PDM_LP);
        }
        // Check if the loop that contains the store is compulsory:
        bool is_loop_comp = pd_tree->dominates(loop_entry, &entry);
        if (is_loop_comp) {
          cp = set_prop(cp, STPR_CMR_LP);
        }
        // See if this loop has a single latch:
        if (BasicBlock *latch_block = inner_loop->getLoopLatch()) {
          cp = set_prop(cp, STPR_SNG_LT);
          if (dm_tree->dominates(parent_block, latch_block)) {
            cp = set_prop(cp, STPR_DOM_LT);
          }
        }
      }
      // Get the number of successors of the store's BB:
      const int num_succs = parent_block->getTerminator()->getNumSuccessors();
      if (num_succs > 1) {
        cp = set_prop(cp, STPR_MUL_SC);
      }
      // Get the number of predecessors of the store's BB:
      const int num_preds = num_predecessors(parent_block);
      if (num_preds > 1) {
        cp = set_prop(cp, STPR_MUL_PD);
      }
      this->worklist[store] = cp;
    }
  }
}


void CFGPropAnalysis::dump() const {
  // Print all the instructions first:
  for (auto pair : this->worklist) {
    errs() << *pair.first << '\n';
  }
  // Now, print their abstract state:
  errs() << get_cfg_feature_names() << '\n';
  for (auto pair : this->worklist) {
    errs() << cfgProp2str(pair.second) << '\n';
  }
}


char CFGPropAnalysis::ID = 0;
static RegisterPass<CFGPropAnalysis> X("cfgPropAnalysis", "CFG properties.");
