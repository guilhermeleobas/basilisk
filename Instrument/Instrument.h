#pragma once

using namespace llvm;

class Instrument : public ModulePass {
  public: 
  // Pass identifier, for LLVM's RTTI support:
  static char ID;

  bool runOnModule(Module&);

  void print_instructions(Module &M);

  Value* Alloc_string_space(Module &M, const std::string str, Instruction *I);

  Function* build_call(Module &, const std::string);

  const std::string get_function_name(Instruction*);
  
  template<typename T>
  void insert_call(Module &M, T *inst){
    IRBuilder<> Builder(inst);

    // Let's create the function call
    // const std::string function_name = get_function_name(inst);
    Function *f = build_call(M, inst);

    Builder.CreateCall(f, std::vector<Value*>());
  }
  

  Instrument() : ModulePass(ID) {}
  ~Instrument() { }

};
