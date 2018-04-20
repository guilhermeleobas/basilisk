#ifndef STRIDE_ANALYSIS_H
#define STRIDE_ANALYSIS_H


#include <map>
#include "Strides.h"

using namespace llvm;

/**
 * This class implements a simple analysis to collect information related to the
 * strides covered by pointers.
 */
class StrideAnalysis : public FunctionPass {

  public:
 
    // Class identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. See if we can erase it.
    StrideAnalysis() : FunctionPass(ID) {}
    ~StrideAnalysis() {}

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
     * @param lp_inf: the result of LoopInfo analysis.
     * @param scalar_ev: the result of scalar evolution analysis.
     */
    void collect_properties(
        Function &f,
        LoopInfo &lp_inf,
        ScalarEvolution &scalar_ev
    );

    /**
     * This method returns the list of properties associated with a store.
     * @return the CFGPropertyWord that corresponds to this value.
     */
    StridePropWord get_properties(const StoreInst* store) const {
      auto tag = worklist.find(store);
      assert(tag != worklist.end() && "Untracked store instruction");
      return tag->second;
    }

  private:

    // This map records Stride properties associated with a store instruction.
    std::map<const StoreInst*, StridePropWord> worklist;

};

#endif
