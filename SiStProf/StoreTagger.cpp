#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"        // To print error messages.
#include "llvm/ADT/Statistic.h"        // For the STATISTIC macro.
#include "llvm/IR/InstIterator.h"      // To use the iterator instructions(f)
#include "llvm/IR/Instructions.h"      // To have access to the Instructions.
#include "llvm/IR/Constants.h"         // For ConstantData, for instance.
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"  // For dbgs()
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfoMetadata.h" // For DILocation
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"


#include <stdio.h>

#include "Flooder.h"
#include "CFGPropAnalysis.h"
#include "ModAnalysis.h"
#include "StrideAnalysis.h"
#include "ValOriginAnalysis.h"
#include "PointerTracker.h"
#include "StoreTagger.h"

#define DEBUG_TYPE "StoreTagger"


STATISTIC(NumTagSts, "Number of tagged store instructions.");


void StoreTagger::getAnalysisUsage(AnalysisUsage &analyses) const {
  analyses.addRequired<LoopInfoWrapperPass>();
  analyses.addRequired<ScalarEvolutionWrapperPass>();
  analyses.addRequired<DominatorTreeWrapperPass>() ;
  analyses.addRequired<PostDominatorTree>();
//  analyses.addRequired<InterProceduralRA<Cousot> >();
  analyses.setPreservesAll();
}


bool StoreTagger::runOnModule(Module &m) {
  unsigned counter = 0;
//  InterProceduralRA<Cousot> &ra =
//    getAnalysis<InterProceduralRA<Cousot>>();
  for (Function &f : m) {
    DEBUG(dbgs() << "Function name " << f.getName() << "\n");
    if (!f.isDeclaration() && !f.empty()) {
      std::vector<const StoreInst*> stores;
      for (Instruction &i : instructions(f)) {
        if(StoreInst *store = dyn_cast<StoreInst>(&i)) {
          process_store(counter++, store);
          stores.push_back(store);
        }
      }
      this->record_structural_properties(f, stores);
      this->record_modifiers(f, stores);
      this->record_val_origins(f, stores);
      this->record_pointer_information(f, stores);
      this->record_stride_properties(f, stores);
      // this->record_ranges(f, ra, stores);
    }
  }
  DEBUG(dbgs() << "Store tagging:\n"; this->dump());
  print_to_file();
  return false;
}


void StoreTagger::record_val_origins(
    const Function &f,
    const std::vector<const StoreInst*> &stores
) {
  ValOriginAnalysis voa;
  voa.compute_origins(f);
  for (const StoreInst *store : stores) {
    Feature *ft = this->get_features(store);
    ft->vori = voa.get_origin(store);
  }
}

/*
void StoreTagger::record_ranges(
  Function &f,
  InterProceduralRA<Cousot> &ra,
  const std::vector<const StoreInst*> &stores
) {
  DEBUG(dbgs() << "\nCousot Inter Procedural analysis (Values -> Ranges) of "
      << f.getName() << ":\n");
  for (const StoreInst *store : stores) {
    DEBUG(dbgs() << "Analysing " << *store << '\n');
    Range r = ra.getRange(store->getValueOperand());
    if (!r.isUnknown()) {
      Feature *ft = this->get_features(store);
      const APInt size = r.getUpper() - r.getLower();
      uint64_t num_values = size.getLimitedValue();
      DEBUG(dbgs() << "Analysing " << *store << '\n');
      DEBUG(dbgs() << "NumValues = " << num_values << '\n');
      if (num_values == 0) {
        ft->rngs = RANGE_N;
      } else if (num_values == 1) {
        ft->rngs = RANGE_1;
      } else if (num_values == 2) {
        ft->rngs = RANGE_2;
      } else if (num_values <= 8) {
        ft->rngs = RANGE_8;
      } else {
        ft->rngs = RANGE_N;
      }
    }
  }
}
*/

void StoreTagger::record_modifiers(
    const Function &f,
    const std::vector<const StoreInst*> &stores
) {
  ModAnalysis ma;
  ma.run_dataflow_analysis(f);
  for (const StoreInst *store : stores) {
    Feature *ft = this->get_features(store);
    ft->value_modifiers = ma.get_status(store->getValueOperand());
  }
}


void StoreTagger::record_stride_properties(
    Function &f,
    const std::vector<const StoreInst*> &stores
) {
  LoopInfo &loop_info =
    getAnalysis<LoopInfoWrapperPass>(f).getLoopInfo();
  ScalarEvolution &scalar_ev = 
    getAnalysis<ScalarEvolutionWrapperPass>(f).getSE();
  StrideAnalysis stride_analysis;
  stride_analysis.collect_properties(f, loop_info, scalar_ev);
  for (const StoreInst *store : stores) {
    Feature *ft = this->get_features(store);
    ft->scev = stride_analysis.get_properties(store);
  }
}


void StoreTagger::record_structural_properties(
    Function &f,
    const std::vector<const StoreInst*> &stores
) {
  const LoopInfo &loop_info =
    getAnalysis<LoopInfoWrapperPass>(f).getLoopInfo();
  const DominatorTree& dm_tree =
    getAnalysis<DominatorTreeWrapperPass>(f).getDomTree();
  const PostDominatorTree& pd_tree = getAnalysis<PostDominatorTree>(f);
  CFGPropAnalysis cfg_a;
  cfg_a.collect_properties(f, &loop_info, &dm_tree, &pd_tree);
  for (const StoreInst *store : stores) {
    Feature *ft = this->get_features(store);
    ft->cfgp = cfg_a.get_properties(store);
  }
}


void StoreTagger::record_pointer_information(
    Function &f,
    const std::vector<const StoreInst*> &stores
) {
  PointerTracker pt;
  pt.compute_origins(f);
  for (const StoreInst *store : stores) {
    Feature *ft = this->get_features(store);
    ft->pointer_origin = pt.get_origin(store);
    ft->typ2 = pt.get_second_level_ptr_type(store);
  }
}


/**
 * This method returns the line in which some instructione exits in the
 * source code.
 * @param i: the instruction whose line we are probing.
 * @return an integer that denotes the line number of the instruction.
 */
int get_line_no(const Instruction *i) {
  if (i) {
    if (MDNode *md_node = i->getMetadata("dbg")) {
      if (DILocation *di_loc = dyn_cast<DILocation>(md_node)) {
        return di_loc->getLine();
      }
    }
  }
  return -1;
}


TypeSize StoreTagger::get_size_bucket(const Value *v) const {
  const Type* st_type;
  if (const CastInst *cast = dyn_cast<CastInst>(v)) {
    st_type = cast->getSrcTy();
  } else {
    st_type = v->getType();
  }
  unsigned size = st_type->getPrimitiveSizeInBits();
  if (size == 0) {
    return SIZE_0;
  } else if (size == 1) {
    return SIZE_1;
  } else if (size <= 8) {
    return SIZE_8;
  } else if (size <= 16) {
    return SIZE_16;
  } else if (size <= 32) {
    return SIZE_32;
  } else {
    return SIZE_N;
  }
}


void StoreTagger::process_store(const int id, const StoreInst *store) {
  NumTagSts++;
  Feature *f = new Feature();
  this->tags[store] = f;
  // Fill up the identifier of the store:
  f->idtf = id;
  // Get the line number, for debugging purpuses:
  f->line = get_line_no(store);
  // Get the type of the store:
  const Type *st_type = store->getValueOperand()->getType();
  f->type = st_type->getTypeID();
  // Get the size of the type:
  f->sztp = get_size_bucket(store->getValueOperand());
  // Get the type of the pointer used in the store:
  const PointerType *ptp =
    dyn_cast<PointerType>(store->getPointerOperand()->getType());
  assert(ptp->getElementType()->getTypeID() == f->type &&
      "Origin and destination of store have different types.");
}


const std::string get_file_name(const Instruction *i) {
  if (i) {
    if (MDNode *md_node = i->getMetadata("dbg")) {
      if (DILocation *di_loc = dyn_cast<DILocation>(md_node)) {
        return di_loc->getFilename();
      }
    }
  }
  return "NO_FILE";
}


void StoreTagger::print_to_file() const {
  size_t num_records = this->tags.size();
  Feature **fs = (Feature**)malloc(num_records * sizeof(Feature*));
  const StoreInst **ss = (const StoreInst**)
    malloc(num_records * sizeof(StoreInst*));
  // Read all the elements into the array, to get them in sorted order.
  for (auto pair : this->tags) {
    fs[pair.second->idtf] = pair.second;
    ss[pair.second->idtf] = pair.first;
  }
  // TAG_FILE_NAME and DEBUG_FILE_NAME are defined in 'Features.h'
  FILE *txt_file = fopen(DEB_FILE_NAME, "w");
  FILE *bin_file = fopen(TAG_FILE_NAME, "wb");
  // Print the number of records into the binary file:
  DEBUG( dbgs() << "\nPrinting " << num_records << " in profiling file\n" );
  fwrite(&num_records, sizeof(size_t), 1, bin_file);
  // Go over the array, printing all the elements into the file:
  for (unsigned long i = 0; i < num_records; i++) {
    fwrite(fs[i], sizeof(Feature), 1, bin_file);
    fprintf(txt_file, "%d, %s\n", fs[i]->idtf, get_file_name(ss[i]).c_str());
  }
  fclose(bin_file);
  fclose(txt_file);
  free(fs);
  free(ss);
}


void StoreTagger::dump() const {
  for (auto pair : this->tags) {
    int id = pair.second->idtf;
    int ln = pair.second->line;
    const StoreInst *st = pair.first;
    const std::string fname = get_file_name(st);
    const Value *vl = st->getValueOperand();
    Type *tp = vl->getType();
    auto nm = vl->hasName() ? vl->getName() : "GlobVr";
    errs() << id << ":[" << ln << " at " << fname << "] " << nm << ", TP = "
      << *tp << '\n';
  }
  errs() << "\n";
}


char StoreTagger::ID = 0;
static RegisterPass<StoreTagger> X("stTag",
    "Store tagging");
