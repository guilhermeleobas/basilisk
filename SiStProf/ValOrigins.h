#ifndef _VORIGINS_H
#define _VORIGINS_H


typedef enum {
  VORI_CONS_ZER = 0, // ZER
  VORI_CONS_INT,     // INT
  VORI_CONS_FLP,     // INT
  VORI_CONS_GLB,     // GLB
  VORI_CONS_ANY,     // CN?
  VORI_ARGUMENT,     // ARG
  VORI_INST_LOA,     // LD
  VORI_INST_CAL,     // CAL
  VORI_INST_MUX,     // PHI or SELECT
  VORI_INST_ADD,     // ADD or SUB
  VORI_INST_INC,     // ADD or SUB by 1
  VORI_INST_NEG,     // Numeric negation.
  VORI_INST_NOT,     // Boolean not.
  VORI_INST_MAD,     // ADD in which one of the operands is a multiplication.
  VORI_INST_MUL,     // MUL or DIV or REM
  VORI_INST_BIN,     // Other binary instructions
  VORI_INST_CAS,     // Casts
  VORI_INST_ALL,     // ALL
  VORI_INST_UNR,     // Unary instructions
  VORI_INST_ANY,     // IN?
  VORI_MAX_MODI
} ValOrigin;

const char* to_str(ValOrigin md);

const char* get_val_origin_names();

#endif
