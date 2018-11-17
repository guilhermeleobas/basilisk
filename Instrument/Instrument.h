#pragma once

using namespace llvm;

class Instrument : public ModulePass {
  public: 
  // Pass identifier, for LLVM's RTTI support:
  static char ID;

  bool runOnModule(Module&);

  // Creates (if not exists) a global string ptr with
  // the instructino opcode name
  Value* get_or_create_string(Instruction *I);
  
  // Create the call to dump the csv to a file
  void insert_dump_call(Module &M, Instruction *I);

  /*
    Add an external call to @count_instruction.
    @param Module is self-explanatory
    @param Instruction is the instruction we want to count

    `count_instruction` is defined in the file Collect/collect.c
  */
  void insert_call(Module &M, Instruction *inst);

  // Get the number of predecessors of a basic block
  int getNumPredecessors(BasicBlock *BB);

  Instrument() : ModulePass(ID) {}
  ~Instrument() { }

};
