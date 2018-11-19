#pragma once

#define FILENAME "count.csv"


typedef struct Instruction{
  char type[20];
  unsigned long long counter;
} Instruction;

#define assertf(A, M, ...) if(!(A)) \
 { fprintf(stderr, M, ##__VA_ARGS__); fprintf(stderr, "\n"); assert(A); }

#define OPCODES_LENGTH 26

static Instruction array[OPCODES_LENGTH] = {
  // Memory Access
  {"store", 0},
  {"load", 0},
  
  // Branches
  {"br", 0},
  {"indirect_br", 0},
  
  // Binary Operators
  {"add", 0},
  {"fadd", 0},
  {"sub", 0},
  {"fsub", 0},
  {"mul", 0},
  {"fmul", 0},
  {"udiv", 0},
  {"sdiv", 0},
  {"fdiv", 0},
  {"urem", 0},
  {"srem", 0},
  {"frem", 0},
  
  // Logical Operators
  {"and", 0},
  {"or", 0},
  {"xor", 0},
  
  // Other Instructions
  {"icmp", 0},
  {"fcmp", 0},
  {"call", 0},
  {"select", 0},
  {"shl", 0},
  {"lshr", 0},
  {"ashr", 0},
  
};

void count_instruction(char *type);
void dump_csv();
