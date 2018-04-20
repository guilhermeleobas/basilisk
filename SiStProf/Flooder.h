#ifndef _FLOODER_H
#define _FLOODER_H


#include <set>

/**
 * This class implements a simple dataflow analysis based on graph reachability.
 * This dataflow analysis computes Interesting nodes, which we define as
 * follows:
 * - A set S of nodes, e.g., the initialization set, is Interesting.
 * - A node computed as function of Interesting nodes is Interesting.
 */
class Flooder  {
  public:

    // Class constructor.
    Flooder(const llvm::Function &f) : function(f) {}

    // Class destructor. Deletes the map of tags.
    ~Flooder() { interesting_values.clear(); }

    /**
     * This method inserts a new node in the set of interesting nodes.
     * @param seed: the new node that shall be marked as interesting.
     */
    void add_interesting(const llvm::Value *seed) {
      this->interesting_values.insert(seed);
    }

    /**
     * This method reports if a value is an interesting node or not.
     * @param v: the node that we are checking.
     * @return true if the target node is interesting, and false otherwise.
     */
    bool is_interesting(const llvm::Value *v) const {
      return this->interesting_values.find(v) != this->interesting_values.end();
    }

    /**
     * This method computes interesting values. It keeps iterating the
     * definition of interesting nodes, until we reach a fixed point.
     */
    void compute_interesting_values();

    /**
     * Boilerplate methods to iterate over instances of this class.
     */
    std::set<const llvm::Value*>::iterator begin() {
      return this->interesting_values.begin();
    }
    std::set<const llvm::Value*>::const_iterator begin() const {
      return this->interesting_values.begin();
    }
    std::set<const llvm::Value*>::const_iterator cbegin() const {
      return this->interesting_values.cbegin();
    }
    std::set<const llvm::Value*>::iterator end() {
      return this->interesting_values.end();
    }
    std::set<const llvm::Value*>::const_iterator end() const {
      return this->interesting_values.end();
    }
    std::set<const llvm::Value*>::const_iterator cend() const {
      return this->interesting_values.cend();
    }

  private:

    // This set contains the interesting values.
    std::set<const llvm::Value*> interesting_values;

    // The LLVM function that is the target of the dataflow analysis:
    const llvm::Function& function;

};

#endif
