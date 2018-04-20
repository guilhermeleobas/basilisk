#ifndef _STRIDES_H
#define _STRIDES_H


typedef enum {
  SCEV_REC = 0,  // Erc: the pointer is created by a recurrence relation.
  SCEV_AFF,  // Eaf: the pointer is formed by an affine expression.
  SCEV_QUA,  // Eqd: the pointer is formed by a quadratic affine expression.
  SCEV_ST1,  // Es1: the pointer has a constant step of 1.
  SCEV_ST4,  // Es4: the pointer has a constant step of 4.
  SCEV_ST8,  // Es8: the pointer has a constant step of 8.
  SCEV_STN,  // EsN: the pointer has a constant step greater than 8.
  SCEV_SPT,  // Esp: the step of the pointer equals the size of the pointer.
  SCEV_TPC,  // Etp: the pointer exists in a loop with known trip count.
  SCEV_MAX
} StrideProp;

typedef unsigned int StridePropWord;

const char* strideProp2str(StridePropWord stride_prop);

const char* get_stride_feature_names();

StridePropWord
set_stride_prop(const StridePropWord prop, const StrideProp new_cp);

StridePropWord get_initial_stride_prop();

#endif
