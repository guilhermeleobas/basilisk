#ifndef _PTR_ORIGINS_H
#define _PTR_ORIGINS_H


/**
 * The possible origins of data:
 * - STATIC: global variables, or data marked as 'static'.
 * - STACK: data created with alloca instructions.
 * - HEAP: data created with heap management routines, e.g., malloc, etc
 * - UNKNOWN: data whose origin we cannot determine. We have further
 *   divided this kind of data into the following subcategories:
 *   - ARGM: addresses that comes as function arguments.
 *   - LOAD: addresses that comes from load operations.
 *   - FUNC: addresses that comes as the return value of functions.
 *   - PHIN: the pointer comes from a phi-function.
 * - NULL_PTR: the null pointer. It will be wrong to store something in
 *   the null pointer, but we can have that in the code. See, for
 *   instance, the function BZ2_bzWriteOpen, in SPEC bzip2.
 * - UNINITIALIZED: a pointer that has not been initialized.
 * - UNK_BIT: some bitwise operation used to manipulate the pointer, as in
 *   "(QUAD_EDGE) ((uptrint) e ^ ((uptrint) e & ANDF))", in olden/voronoi.
 */
typedef enum {
  STATIC = 0,      // Msc
  STACK,           // Msk
  HEAP,            // Mhp
  UNK_ARGM,        // Mar
  UNK_LOAD,        // Mld
  UNK_FUNC,        // Mfn
  UNK_PHIN,        // Mph
  NULL_PTR,        // Mpt
  UNINITIALIZED,   // Mun
  UNK_BIT,         // Mbt
  PTR_OR_MAX
} PtrOrigin;

#endif
