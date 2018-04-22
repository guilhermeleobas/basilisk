#pragma once

using namespace llvm;

class Instrument : public ModulePass {
  public: 
  // Pass identifier, for LLVM's RTTI support:
  static char ID;

  bool runOnModule(Module&);

  void print_instructions(Module &M);

  Value* Alloc_string_space(Module &M, const std::string str, Instruction *I);

  const std::string get_function_name(Instruction*);

  Function* build_call(Module &, const std::string &);
  
  void insert_call(Module &M, Instruction *inst);

  Instrument() : ModulePass(ID) {}
  ~Instrument() { }

};
