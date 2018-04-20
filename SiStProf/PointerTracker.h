#ifndef _POINTER_TRACKER_H
#define _POINTER_TRACKER_H


#include <map>

#include "PtrOrigins.h"

using namespace llvm;

/**
 * This class contains methods to track the origin of pointers. There are four
 * possible origins: stack, static, heap and unknown (mostly the result of
 * load operations or function calls).
 */
class PointerTracker {
  public:

    // Class constructor. Only here to initialize the pass identifier.
    PointerTracker() {}

    // Class destructor. Deletes the map of origins.
    ~PointerTracker() {
      this->origins.clear();
    }

    /**
     * Outputs in the standard error output data related to the store
     * instructions that have been tracked.
     */
    void dump() const;

    /**
     * This method returns the origin of a store instruction.
     * @param store: the instruction that we want to analyze.
     * @return an PtrOrigin data type, that identifies the origin of the store.
     */
    PtrOrigin get_origin(const StoreInst* store) const {
      auto tag = origins.find(store);
      assert(tag != origins.end() && "Untracked store instruction");
      return tag->second;
    }

    /**
     * This method returns the type of the second-level pointer used in this
     * store operation. The level of a pointer is given by the number of geps
     * used to build that pointer. Here, we stop the search once we find the
     * type of the pointer used in the first gep instruction. That means that
     * we return the following types for the following store operations:
     * *a = 3 => int*
     * a->b = 3 => struct*
     * a[b]->c = 3 => struct* // We do not track the third level pointer.
     * @param store: the instruction that we want to analyze.
     * @return the type ID of the second-level pointer. If this pointer has
     * only one level, then the type ID of that level.
     */
    unsigned get_second_level_ptr_type(const StoreInst* store) const {
      auto tag = ptr_type.find(store);
      assert(tag != ptr_type.end() && "Untracked store instruction");
      return tag->second;
    }

    /**
     * Compute the origin of store operations.
     * @param f: the function that shall be analyzed. 
     */
    void compute_origins(Function &f);

  private:

    // Maps stores to the origin of the memory region they write
    std::map<const StoreInst*, PtrOrigin> origins;

    // Map stores to the type of the 2-level pointer they use: struct, array...
    std::map<const StoreInst*, unsigned> ptr_type;

    /**
     * This function keeps a list of which functions allocate memory in the
     * heap.
     */
    bool allocates_heap(const CallInst *call) const ;

    /**
     * Update the statistics related to the origin of stores.
     * @param origin: a value that determines which stat var will be updated.
     */
    void update_stats(const PtrOrigin origin);

    /**
     * This method tracks the origin of a single store instruction.
     * @param v: the value that leads to the origin of some store instruction.
     */
    PtrOrigin track_origin(Value* v) const;

    /**
     * This method searches the type of the target of a store instruction, which
     * can be a struct, an array, a scalar, etc.
     * @param v: the value whose type we want to find out.
     * @return the type id of the pointer (See LLVM's enum TypeID).
     */
    unsigned track_ptr_type(Value *v) const;

};

#endif
