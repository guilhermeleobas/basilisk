#ifndef MOD_ANALYSIS_H
#define MOD_ANALYSIS_H


#include <map>
#include "Modifiers.h"

using namespace llvm;

/**
 * This class implements a simple analysis to determine how values are
 * contituted. In other words, it determines, for each value in a function F,
 * the type of the other operations that bear an influence on that value.
 */
class ModAnalysis : public FunctionPass {

  public:
 
    // Class identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. See if we can erase it.
    ModAnalysis() : FunctionPass(ID) {}
    ~ModAnalysis() {this->worklist.clear();}

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
    void run_dataflow_analysis(const Function &f);

    // This method returns the abstract state associated with some value.
    // @param v: the value that we are querying.
    // @return the ModifierWord that corresponds to this value.
    // If the value has not been processed thus far (e.g., is the argument of
    // a phi-function), then we return the initial modifier word.
    ModifierWord get_status(const Value* v) const;

  private:

    // This map stores the status of the values in the function.
    std::map<const Instruction*, ModifierWord> worklist;

    // This method returns the Modifier that corresponds to some LLVM value.
    // In practice, it implements an LLVM to Modifier converter.
    // @param i: the instruction that we are querying. Notice that this method
    // can only be invoked onto instructions.
    // @return the Modifier key that corresponds to the type of i.
    Modifier get_modifier(const Instruction &i) const;

};

#endif
