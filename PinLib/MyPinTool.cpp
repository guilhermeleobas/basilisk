#include <iomanip>
#include <iostream>
#include <set>
#include <map>
#include <string>

#include "pin.H"
#include "instlib.H"

using namespace INSTLIB;

ofstream out;
FILTER filter;
// KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "pin.out",
//                             "specify output file name");

static std::map<std::string, unsigned long long int> mapa;
static bool valid = true;

void count_stores(const string *s){
  if (valid){
    mapa["store_" + *s]++;
  } 
}

void count_branches(const string *s, bool is_direct){
  if (valid){
    if (is_direct){
      mapa["br_" + *s]++;
    }
    else {
      mapa["indirectbr_" + *s]++;
    }
  }
}

void count_loads(const string *s){
  if (valid){
    mapa["load_" + *s]++;
  }
}

void count_binary_inst(const string *s){
  if (valid){
    
  }
}

VOID mark_start(){
  valid = false;
}

VOID mark_end(){
  valid = true;
}

VOID Image(IMG img, VOID *v){
    RTN rtn = RTN_FindByName(img, "count_instruction");
    if (RTN_Valid(rtn)){
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mark_start, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)mark_end, IARG_END);
        RTN_Close(rtn);
    }

    RTN rtn2 = RTN_FindByName(img, "dump_csv");
    if (RTN_Valid(rtn2)){
        RTN_Open(rtn2);
        RTN_InsertCall(rtn2, IPOINT_BEFORE, (AFUNPTR)mark_start, IARG_END);
        RTN_InsertCall(rtn2, IPOINT_AFTER, (AFUNPTR)mark_end, IARG_END);
        RTN_Close(rtn2);
    }
}

VOID Trace(TRACE trace, VOID *a) {
  // if (!filter.SelectTrace(trace))
    // return;

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      // out << INS_IsBranch(ins) << ' ' << INS_IsDirectBranch(ins) << ' ' << INS_IsDirectBranchOrCall(ins) << ' ' << INS_IsIndirectBranchOrCall(ins) << ' ' << INS_Disassemble(ins) << "\n";
      //
      // if (INS_IsBranch(ins)){
      //   bool direct_branch = false;
      //
      //   if (INS_IsDirectBranch(ins))
      //     direct_branch = true;
      //
      //   INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)count_branches,
      //       IARG_PTR, new string(INS_Mnemonic(ins)),
      //       IARG_BOOL, direct_branch,
      //       IARG_END);
      // }

      // if (INS_IsMemoryWrite(ins)){
      //   INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)count_stores,
      //       IARG_PTR, new string(INS_Mnemonic(ins)),
      //       IARG_END);
      // }

      if (INS_IsMemoryRead(ins)){
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)count_loads,
            IARG_PTR, new string(INS_Mnemonic(ins)),
            IARG_END);
      }

    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  bool go = false;
  for (map<const string, unsigned long long int>::iterator it = mapa.begin();
       it != mapa.end(); it++){
    if (go)
      out << ',' << it->first;
    else
      out << it->first;

    go = true;
  }
  out << endl;
  
  go = false;
  for (map<const string, unsigned long long int>::iterator it = mapa.begin();
       it != mapa.end(); it++){
    if (go)
      out << ',' << it->second;
    else
      out << it->second;

    go = true;
  }

  out << endl;
  out.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage() {
  cerr << "This pin tool demonstrates use of FILTER to identify "
          "instrumentation points\n"
          "\n";

  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  out.open("pin.csv");

  filter.Activate();

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
