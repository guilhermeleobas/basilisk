#ifndef VAL_ORIGIN_ANALYSIS_H
#define VAL_ORIGIN_ANALYSIS_H

#include <map>
#include "ValOrigins.h"


using namespace llvm;

/**
 * This class implements a simple analysis to determine the origin of the
 * values stored in memory. The query is O(1) for each store instruction.
 */
class ValOriginAnalysis : public FunctionPass {

  public:
 
    // Class identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. See if we can erase it.
    ValOriginAnalysis() : FunctionPass(ID) {}
    ~ValOriginAnalysis() {this->worklist.clear();}

    // Methods overwritten from the FunctionPass class:
    void getAnalysisUsage(AnalysisUsage &) const override;
    virtual bool runOnFunction(Function &) override;

    /**
     * Outputs in the standard error output the abstract state of each
     * program value.
     */
    void dump() const;

    // This method runs the dataflow analysis onto function f. I've extracted
    // it from runOnFunction to be able to use this machinary as a separate
    // object.
    // @param f: the function that is about to be analyzed.
    void compute_origins(const Function &f);

    // This method returns the origin of the value used in a store instruction.
    // @param store: the instruction that we are querying.
    // @return the ValOrigin that corresponds to this value.
    ValOrigin get_origin(const StoreInst* store) const {
      auto tag = worklist.find(store);
      assert(tag != worklist.end() && "Untracked store instruction");
      return tag->second;
    }

  private:

    // This map stores the origin of the values used in store instructions.
    std::map<const StoreInst*, ValOrigin> worklist;

    // This method returns the ValOrigin that corresponds to some LLVM value.
    // In practice, it implements an LLVM to ValOrigin converter.
    // @param v: the value that we are querying.
    // @return the ValOrigin key that corresponds to the type of i.
    ValOrigin val2origin(const Value *v) const;

};

#endif
