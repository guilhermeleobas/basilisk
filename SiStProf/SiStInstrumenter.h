#ifndef _INSTRUMENTATION_H
#define _INSTRUMENTATION_H

#include <set>


using namespace llvm;

/**
 * This class contains methods to instrument the store instructions in an LLVM
 * file. This instrumentation allows us to profile the contents of that
 * instruction. In particular, we can check if the store was silent. A silent
 * store is a store operation that records in memory the same value that was
 * already there; hence, it does not case a change of state in the machine.
 */
class SiStInstrumenter : public FunctionPass {
  public:

    // Pass identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. Only here to initialize the pass identifier.
    SiStInstrumenter() : FunctionPass(ID), st_tagger(NULL) {}
    ~SiStInstrumenter() { }

    // Methods overwritten from the FunctionPass class:
    void getAnalysisUsage(AnalysisUsage &) const override;
    virtual bool runOnFunction(Function &) override;

    /**
     * Outputs in the standard error output the store instructions that have
     * been instrumented.
     */
    void dump() const;

  private:

    // This set contains the store instructions that have been instrumented.
    std::set<const StoreInst*> worklist;

    // This class associates stores with unique identifiers.
    StoreTagger *st_tagger;

    /**
     * This method indicates if we can instrument a store instruction or not.
     * Currently, we do not instrument stores that operate on vector types.
     * @param store: the store that we are verfying.
     * @return true if we can instrument it, and false otherwise.
     */
    bool is_instrumentable(StoreInst *store) const;

    /**
     * This method inserts the profiling instrumentation before the store
     * instruction that it receives as an argument.
     * @param f: the function where instrumentation will be inserted.
     * @param store: this instruction that will be instrumented.
     */
    void insert_instrumentation(Function &f, StoreInst* store);

    /**
     * This method creates a load instructoin with the same address of the
     * store given as argument. It then inserts a comparison between the value
     * loaded, and the value to be stored. The result of this comparison is
     * returned. This comparison lets us verify the value
     * in the target address of the store, to check if the store is silent.
     * @param f: the function where instrumentation will be inserted.
     * @param store: the instruction containing the address to be checked.
     * @return the result of the comparison that has been just created.
     */
    Value* insert_store_query(Function &f, StoreInst* store);

    /**
     * This method inserts code to log in the data of a store, right after that
     * instruction. This code is a call of a function to do the logging.
     * @param f: the function where instrumentation will be inserted.
     * @param load: the value that is in the target of the store.
     * @param store: the instruction that is profiled.
     */
    void insert_logging_function
      (Function &f, Value* comp_result, StoreInst* store);

};

#endif
