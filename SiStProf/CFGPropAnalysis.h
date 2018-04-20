#ifndef AFFINE_ANALYSIS_H
#define AFFINE_ANALYSIS_H


#include <map>
#include "CFGProperties.h"

using namespace llvm;

/**
 * This class implements a simple analysis to collect structural properties of
 * the Control Flow Graph.
 */
class CFGPropAnalysis : public FunctionPass {

  public:
 
    // Class identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. See if we can erase it.
    CFGPropAnalysis() : FunctionPass(ID) {}
    ~CFGPropAnalysis() {this->worklist.clear();}

    // Methods overwritten from the FunctionPass class:
    void getAnalysisUsage(AnalysisUsage &) const override;
    virtual bool runOnFunction(Function &) override;

    /**
     * Outputs in the standard error output the abstract state of each
     * program value.
     */
    void dump() const;

    /**
     * This method runs the dataflow analysis onto function f. I've extracted
     * it from runOnFunction to be able to use this machinary as a separate
     * object.
     * @param f: the function that is about to be analyzed.
     * @param loop_info: the result of LoopInfo analysis.
     * @param dm_tree: the dominance tree of the CFG.
     * @param pd_tree: the post-dominance tree of the CFG.
     */
    void collect_properties(
        const Function &f,
        const LoopInfo *loop_info,
        const DominatorTree *dm_tree,
        const PostDominatorTree *pd_tree
    );

    // This method returns the list of properties associated with a store.
    // @return the CFGPropertyWord that corresponds to this value.
    CFGPropertyWord get_properties(const StoreInst* store) const {
      auto tag = worklist.find(store);
      assert(tag != worklist.end() && "Untracked store instruction");
      return tag->second;
    }

  private:

    // This map records CFG properties associated with a store instruction.
    std::map<const StoreInst*, CFGPropertyWord> worklist;

};

#endif
