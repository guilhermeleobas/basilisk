#pragma once

using namespace llvm;

class Instrument : public ModulePass {
  public: 
  // Pass identifier, for LLVM's RTTI support:
  static char ID;

  bool runOnModule(Module&);

  void print_instructions(Module &M);
  GlobalVariable* create_global(Module&);
  void insert_call(Module&, GlobalValue*, Instruction*);
  Value* increment(Module&, GlobalVariable*, Instruction*);

  Instrument() : ModulePass(ID) {}
  ~Instrument() { }

};
