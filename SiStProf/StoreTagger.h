#ifndef _STORE_TAGGER_H
#define _STORE_TAGGER_H

#include <map>

// We need to include Features.h, because some implementations, e.g., the
// destructor and get_id use details of the Feature type.
#include "Features.h"
// #include "RangeAnalysis.h"

namespace llvm {
  class Value;
  class LoopInfo;
  class DominatorTree;
  struct PostDominatorTree;
}

class Flooder;

using namespace llvm;

/**
 * This class contains methods to tag stores. Tagging a store means to find a
 * unique ID to the store. This ID is unique in the program; thus, this pass
 * is implemented as a Module Pass, instead of a Function Pass.
 */
class StoreTagger : public ModulePass {
  public:

    // Pass identifier, for LLVM's RTTI support:
    static char ID;

    // Class constructor. Only here to initialize the pass identifier.
    StoreTagger() : ModulePass(ID) {}

    // Class destructor. Deletes the map of tags.
    ~StoreTagger() {
      for (auto pair : this->tags) {
        delete pair.second;
      }
      tags.clear();
    }

    // Methods overwritten from the FunctionPass class:
    void getAnalysisUsage(AnalysisUsage &) const override;
    virtual bool runOnModule(Module &) override;

    /**
     * Outputs in the standard error output the store instructions that have
     * been tagged, as well as their identifiers.
     */
    void dump() const;

    /**
     * This method returns the identifier of a store instruction.
     * Notice that this method is redundant with get_features. However, we
     * keep it, because we have to use get_id pretty often.
     * @param store: the instruction that we want to identify.
     * @return an integer that represents the identifier of the store.
     */
    int get_id(const StoreInst* store) const {
      auto tag = tags.find(store);
      assert(tag != tags.end() && "Untagged store instruction");
      return tag->second->idtf;
    }

    /**
     * This method returns the feature vector associated with a store.
     * @param store: the instruction that we want to identify.
     * @return a struct Feature object that represents the store.
     */
    Feature* get_features(const StoreInst* store) const {
      auto tag = tags.find(store);
      assert(tag != tags.end() && "Untagged store instruction");
      return tag->second;
    }

    /**
     * This method dumps feature information into two text files. The names
     * of these text files is in Features.h. Thus, notice that this method
     * opens/creates two files.
     */
    void print_to_file() const;

  private:

    // This set contains the store instructions that have been instrumented.
    std::map<const StoreInst*, Feature*> tags;

    /**
     * This method saves a store in the map of tags.
     * @param id: the identifier of the store.
     * @param store: the instruction that shall be saved.
     */
    void process_store(const int id, const StoreInst* store);


    /**
     * This method logs in structural properties related to this store.
     * @param f: the Function that we are analyzing.
     * @param stores: this vector contains the stores in f.
     */
    void record_structural_properties(
      Function &f,
      const std::vector<const StoreInst*> &stores
    );


    /**
     * This method logs in the stride of the pointer used as the target of the
     * store operation.
     * @param f: the Function that we are analyzing.
     * @param stores: this vector contains the stores in f.
     */
    void record_stride_properties(
      Function &f,
      const std::vector<const StoreInst*> &stores
    );


    /**
     * This method logs in flow information related to this store.
     * @param f: the Function that we are analyzing.
     * @param stores: this vector contains the stores in f.
     */
    void record_modifiers(
      const Function &f,
      const std::vector<const StoreInst*> &stores
    );


    /**
     * This method records the origin of the values stored in memory, and
     * associates this information with the store instructions.
     * @param f: the Function that we are analyzing.
     * @param stores: this vector contains the stores in f.
     */
    void record_val_origins(
      const Function &f,
      const std::vector<const StoreInst*> &stores
    );


    /**
     * This method records the origin of the values stored in memory, and
     * associates this information with the store instructions.
     * @param f: the Function that we are analyzing.
     * @param ra: the result of the range analysis.
     * @param stores: this vector contains the stores in f.
     */
    /*
    void record_ranges(
      Function &f,
      InterProceduralRA<Cousot> &ra,
      const std::vector<const StoreInst*> &stores
    );
    */


    /**
     * This method returns the approximate size of the type of this value, in
     * bits.
     * @param v: the value whose size we are probing.
     * @return a TypeSize (defined in Features.h) value.
     */
    TypeSize get_size_bucket(const Value *v) const; 


    /**
     * This method logs in the features related to the pointer used in a store.
     * @param f: the Function that we are analyzing.
     * @param stores: this vector contains the stores in f. We are passing it
     *        to avoid recomputing the list of stores whenever we need to
     *        run a different dataflow analysis.
     */
    void record_pointer_information(
      Function &f,
      const std::vector<const StoreInst*> &stores
    );
};

#endif
