#ifndef _SOURCES_H
#define _SOURCES_H


typedef enum {
  CONST_ZERO = 0,        // ZER
  CONST_INT,             // INT
  CONST_FP,              // FTP
  CONST_NULL,            // NUL
  CONST_GLOBAL,          // GLB
  CONST_FUNCTION,        // FUN
  CONST_UNKNOWN,         // CN?
  ARGUMENT,              // ARG
  INST_LOAD,             // LD
  INST_CALL,             // CAL
  INST_COMP,             // CMP
  INST_PHI,              // PHI
  INST_SELECT,           // SEL
  INST_ADD,              // ADD
  INST_SUB,              // SUB
  INST_MUL,              // MUL
  INST_DIV,              // DIV
  INST_REM,              // REM
  INST_BITWISE_SHF,      // SHF
  INST_BITWISE_LOGICAL,  // BIT
  INST_GEP,              // GEP
  INST_BIT_CAST,         // CAS
  INST_FP_EXT,           // EXT
  INST_FP_TO_INT,        // F2I
  INST_INT_TO_FP,        // I2F
  INST_INT_TO_PTR,       // I2P
  INST_PTR_TO_INT,       // P2I
  INST_TRUNC,            // TRN
  INST_SIGN_EXT,         // SEX
  INST_ZERO_EXT,         // ZEX
  INST_ALLOCA,           // ALL
  INST_UNKNOWN,          // IN?
  MAX_MODIFIER
} Modifier;

typedef unsigned int ModifierWord;

ModifierWord set_modifier(ModifierWord md, Modifier new_mod);

ModifierWord modifier_meet(ModifierWord m1, ModifierWord m2);

ModifierWord get_initial_state();

const char* to_str(ModifierWord md);

const char* get_column_names();

#endif
