#ifndef _CFG_PROPS_H
#define _CFG_PROPS_H


typedef enum {
  STPR_MAINFN = 0,  // Smn: The store exists in the main function.
  STPR_LOOP_0,  // Sl0: The store is not within a loop.
  STPR_LOOP_1,  // Sl1: The store is within a outermost loop.
  STPR_LOOP_2,  // Sl2: The store is within a doubly nested loop.
  STPR_LOOP_3,  // Sl3: The store is within a triply nested loop.
  STPR_LOOP_n,  // Sln: The store is within a more than triply nested loop.
  STPR_CMPSRY,  // Scm: The store is compulsory (post-dominates the function).
  STPR_CMR_LP,  // Scl: The store exists in a loop that is compulsory
  STPR_PDM_LP,  // Spl: The store post-dominates the loop entry point
  STPR_LPEXUN,  // Slx: The store exists in a loop that has only one exit
  STPR_MUL_PD,  // Smp: The store exists in a block with multiple predecessors
  STPR_MUL_SC,  // Sms: The store exists within a block with multiple successors
  STPR_SNG_LT,  // Ssl: The store exists within a block with a single latch.
  STPR_DOM_LT,  // Sdl: The store dominates the single latch.
  STPR_VL_INV,  // Svi: The store uses a loop invariant value.
  STPR_PT_INV,  // Spi: The store uses a loop invariant pointer.
  STPR_MAX
} CFGProperty;

typedef unsigned int CFGPropertyWord;

const char* cfgProp2str(CFGPropertyWord cfg_prop);

const char* get_cfg_feature_names();

CFGPropertyWord set_prop(const CFGPropertyWord md, const CFGProperty new_cp);

CFGPropertyWord get_initial_cfg_prop();

#endif
