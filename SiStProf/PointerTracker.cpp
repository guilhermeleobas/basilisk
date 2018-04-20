#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"        // To extract constants from stores.
#include "llvm/Support/Debug.h"       // To print error messages.
#include "llvm/ADT/Statistic.h"       // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"     // To use the iterator instructions
#include "llvm/IR/Instructions.h"     // To have access to the Instruction cls.

#include "llvm/Support/raw_ostream.h" // For dbgs()

#include "PointerTracker.h"

#define DEBUG_TYPE "PointerTracker"


void PointerTracker::compute_origins(Function &f) {
  DEBUG(dbgs() << "\nProcessing " << f.getName() << "\n");
  for (Instruction &i : instructions(f)) {
    if(StoreInst *store = dyn_cast<StoreInst>(&i)) {
      DEBUG(dbgs() << "\nStore: " << *store << "\n");
      Value *ptr = store->getPointerOperand();
      // Compute the origin of the pointer: stack, heap, static, etc.
      PtrOrigin origin = this->track_origin(ptr);
      this->origins[store] = origin;
      // Compute the type of the pointer: array, struct, int, double, etc
      unsigned ptr_type = this->track_ptr_type(ptr);
      this->ptr_type[store] = ptr_type;
    }
  }
}


bool PointerTracker::allocates_heap(const CallInst *call) const {
  const Function *callee = call->getCalledFunction();
  if (callee && callee->hasName()) {
    const std::string fun_name = callee->getName();
    return fun_name == "malloc" ||
           fun_name == "calloc" ||
           fun_name == "realloc";
  } else {
    return false;
  }
}


PtrOrigin PointerTracker::track_origin(Value* v) const {
  if (isa<GlobalValue>(v)) {
    return STATIC;
  } else if (isa<AllocaInst>(v)) {
    return STACK;
  } else if (isa<InvokeInst>(v)) {                                          
    return UNK_FUNC;
  } else if (const CallInst *call = dyn_cast<CallInst>(v)) {
    if (this->allocates_heap(call)) {
      return HEAP;
    } else {
      return UNK_FUNC;
    }
  } else if (isa<LoadInst>(v)) {
    return UNK_LOAD;
  } else if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(v)) {
    return this->track_origin(gep->getPointerOperand());
  } else if (ConstantExpr *expr = dyn_cast<ConstantExpr>(v)) {
    Instruction* inst = expr->getAsInstruction();
    PtrOrigin origin =  this->track_origin(inst);
    delete inst;
    return origin;
  } else if (isa<ConstantPointerNull>(v)) {
    return NULL_PTR;
  } else if (isa<UndefValue>(v)) {
    return UNINITIALIZED;
  } else if (isa<Argument>(v)) {
    return UNK_ARGM;
  } else if (isa<PHINode>(v) || isa<SelectInst>(v)) {
    return UNK_PHIN;
  } else if (const UnaryInstruction *u_inst = dyn_cast<UnaryInstruction>(v)) {
    return this->track_origin(u_inst->getOperand(0));
  } else {
    // The only thing that is left to return now is some bit operation used
    // to change the pointer, such as
    // "(QUAD_EDGE) ((uptrint) e ^ ((uptrint) e & ANDF))"
    // in olden/voroni/newvor.c.
    return  UNK_BIT;
  }
}


unsigned PointerTracker::track_ptr_type(Value *v) const {
  const PointerType *tp = dyn_cast<PointerType>(v->getType());
  if (isa<Instruction>(v)) {
    if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(v)) {
      return this->track_ptr_type(gep->getPointerOperand());
    } else {
      return tp->getElementType()->getTypeID();
    }
  } else if (ConstantExpr *expr = dyn_cast<ConstantExpr>(v)) {
    Instruction* inst = expr->getAsInstruction();
    unsigned ptr_type = this->track_ptr_type(inst);
    delete inst;
    return ptr_type;
  } else {
    return tp->getElementType()->getTypeID();
  }
}


std::string origin2str(PtrOrigin origin) {
  switch (origin) {
    case STATIC       : return "Static";
    case STACK        : return "Stack";
    case HEAP         : return "Heap";
    case UNK_ARGM     : return "Unknown (argument)";
    case UNK_FUNC     : return "Unknown (function)";
    case UNK_LOAD     : return "Unknown (load)";
    case UNK_PHIN     : return "Unknown (phi)";
    case NULL_PTR     : return "The null pointer";
    case UNINITIALIZED: return "Uninitialized location";
    case UNK_BIT      : return "Unknown bit operation";
    default           : return "Unknown origin";
  }
}


void PointerTracker::dump() const {
  for (auto pair : this->origins) {
    errs() << origin2str(pair.second) << ": " << *pair.first << '\n';
  }
}
