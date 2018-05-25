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

std::set<std::string> sete;

// void count_inst(const string *type, const string *s){
//   if (valid){
//     mapa[*type + "_" + prefix + *s]++;
//   }
// }

void count_inst(const string *type){
  if (valid){
    mapa[*type + "_" + prefix]++;
  }
}


std::string check_mnemonic(const string &m){

  if (m == "ADD" || m == "INC"){
    return "ADD";
  }

  if (m == "SUB" || m == "DEC" || m == "SBB" ||
      m == "PSUBB" || m == "PSUBW" || m == "PSUBD"){
    return "SUB";
  }

  if (m == "MUL" || m == "IMUL"){
    return "MUL";
  }

  if (m == "DIV" || m == "IDIV")
    return "DIV";

  // floating-point instructions
  if (m == "ADDPD" || m == "ADDPS" || m == "ADDSD" || m == "ADDSS" ||
      m == "FADD" || m == "FADDP" || m == "FIADD")
    return "FADD";

  if (m == "SUBPD" || m == "SUBPS" || m == "SUBSD" || m == "SUBSS" ||
      m == "FSUB" || m == "FSUBP" || m == "FISUB")
    return "FSUB";

  if (m == "MULPD" || m == "MULPS" || m == "MULSD" || m == "MULSS" ||
      m == "FMUL" || m == "FMULP" || m == "FIMUL")
    return "FMUL";

  if (m == "DIVPD" || m == "DIVPS" || m == "DIVSD" || m == "DIVSS" ||
      m == "FDIV" || m == "FDIVP" || m == "FIDIV" ||
      m == "FDIVR" || m == "FDIVRP" || m == "FIDIVR")
    return "FDIV";

  return "";
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

  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

      std::string s = check_mnemonic(INS_Mnemonic(ins));

      if (s.size() != 0){
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)count_inst,
            IARG_PTR, new string(s),
            // IARG_PTR, new string(INS_Mnemonic(ins)),
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
  
  out.open("binops.csv");

  // filter.Activate();

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}