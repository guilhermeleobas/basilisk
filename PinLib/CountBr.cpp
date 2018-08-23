#include <iomanip>
#include <iostream>
#include <set>
#include <map>
#include <string>

#include "pin.H"
#include "instlib.H"
#include "lib.H"

using namespace INSTLIB;

ofstream out;
FILTER filter;

void count_inst(const string *type){
  if (valid){
    if (prefix == "before")
      bef[*type]++;
    else if (prefix == "main")
      ma[*type]++;
    else
      en[*type]++;
  }
}

VOID Trace(TRACE trace, VOID *a) {
  // if (!filter.SelectTrace(trace))
  //   return;

  RTN rtn = TRACE_Rtn(trace);
  if (RTN_Valid(rtn)){
    if (RTN_Name(rtn) == "count_instruction" || 
        RTN_Name(rtn) == "dump_csv")
      return;
  }
  
  init_if_not_exists(new string("br"));
  init_if_not_exists(new string("indirect"));

  /* bef["br"] = 0; ma["br"] = 0; en["br"] = 0; */
  /* bef["indirect"] = 0; ma["indirect"] = 0; en["indirect"] = 0; */
  
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      if (INS_IsBranch(ins)){
        if (INS_IsIndirectBranchOrCall(ins)){
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)count_inst,
              IARG_PTR, new string("indirect"),
              IARG_END);
        }
        if (INS_IsDirectBranchOrCall(ins)){
          INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)count_inst,
              IARG_PTR, new string("br"),
              IARG_END);
        }
      }

    }
  }
}

VOID Fini(INT32 code, VOID *v) {
  out << "br_before, br_main, br_end, indirect_before, indirect_main, indirect_end\n";
  out << bef["br"] << ", " << ma["br"] << ", " << en["br"] << ", ";
  out << bef["indirect"] << ", " << ma["indirect"] << ", " << en["indirect"] << "\n";
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
  
  out.open("br.csv");

  // filter.Activate();

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
