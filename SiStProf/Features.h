#pragma once


#include "ValOrigins.h"
#include "PtrOrigins.h"
#include "CFGProperties.h"
#include "Modifiers.h"
#include "Strides.h"

// We always dump the tag data into a file called "feat.dat", a short name
// for 'features'.
const static char* TAG_FILE_NAME = "feat.dat";

// We dump debugging data, i.e., the identifier, line and file of the store
// into a text file with this name:
const static char* DEB_FILE_NAME = "debug.txt";

typedef enum {
  RANGE_1 = 0, // rg1
  RANGE_2,     // rg2
  RANGE_8,     // rg8
  RANGE_N,     // rgN
  RANGE_MAX
} RangeSize;

typedef enum {
  SIZE_0 = 0,   // sz0
  SIZE_1,       // sz1
  SIZE_8,       // sz8
  SIZE_16,      // sz16
  SIZE_32,      // sz32
  SIZE_N,       // szN
  SIZE_TYPE_MAX // szN
} TypeSize;

/**
 * This struct groups the static features that we are mining from programs.
 * To add a new feature, you must:
 * 1. Insert the field in the struct.
 * 2. Update the following methods in store_data_collector.c:
 *    - print_data
 *    - dump (just to add the column in the csv file)
 * 3. Update StoreTagger.(h/cpp) to collect the new feature.
 */
typedef struct {
  int idtf;    // The identifier of the store
  int line;    // The line where the store is located in the source file
  int type;    // The type of the store, e.g., int, float, double, etc
  TypeSize sztp;    // The size of the type's value, when primitive.
  RangeSize rngs;    // The range of values covered by the value, if scalar.
  int typ2;    // The second level type of the store operation.
  ValOrigin vori; // The origin of the value used in the store.
  CFGPropertyWord cfgp; // Structural properties of the CFG.
  StridePropWord scev; // Properties related to the pointer's scalar evolution.
  PtrOrigin pointer_origin;  // The origin of the pointer: STATIC, STACK, HEAP, etc.
  ModifierWord value_modifiers;    // The list of modifiers of the store.
} Feature;
