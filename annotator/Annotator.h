#pragma once

using namespace llvm;

class Annotator : public FunctionPass {
  public: 
  // Pass identifier, for LLVM's RTTI support:
  static char ID;

  bool runOnFunction(Function&);

  Annotator() : FunctionPass(ID) {}
  ~Annotator() { }

};
