#include <iomanip>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <vector>

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

template<typename T, size_t N>
T * end(T (&ra)[N]) {
    return ra + N;
}

void init(){

  const char *ADD[] = {"ADC", "ADD", "ADD_LOCK", "INC", "PADDD", "PADDQ", "XADD_LOCK"};
  const char *FADD[] = {"ADDPD", "ADDPS", "ADDSD", "ADDSS", "FADD", "FADDP", "VADDSD"};
  const char *SUB[] = {"DEC", "DEC_LOCK", "PSUBB", "SBB",
                                          "SUB", "VPSUBB"};  
  const char *FSUB[] = {"FSUB", "FSUBP", "FSUBRP", "SUBPD",
                                           "SUBSD", "SUBSS", "VSUBSD"};
  const char *MUL[] = {"IMUL", "MUL"};
  const char *FMUL[] = {"FMUL", "FMULP", "MULPD", "MULPS",
                                           "MULSD", "MULSS", "VMULSD"};
  const char *DIV[] = {"DIV", "IDIV"};
  const char *FDIV[] = {"DIVPD", "DIVSD", "DIVSS", "FDIV", "VDIVSD"};
  const char *AND[] = {"AND", "ANDNPD", "ANDPD", "ANDPS", "VANDNPD",
                               "VANDPD", "VPAND", "VPANDN"};
  const char *OR[] = {"OR", "OR_LOCK", "POR", "VORPD", "VPOR"};
  const char *CMP[] = {"CMP", "CMPSD_XMM", "CMPSS", "CMPXCHG", "CMPXCHG_LOCK", "PCMPEQB",
      "PCMPEQD", "PCMPISTRI", "PTEST", "REPE_CMPSB", "TEST", "VPCMPEQB",
      "VPCMPGTB"};
  const char *FCMP[] = {"FUCOMIP", "UCOMISD", "UCOMISS",
                                           "VCMPSD", "VUCOMISD"};
  const char *SHL[] = {"PSLLDQ", "SHL", "SHLD"};
  const char *ASHR[] = {"SAR"};
  const char *LSHR[] = {"PSRLDQ", "SHR", "SHRD"};
  const char *CALL[] = {"CALL_NEAR", "SYSCALL"};
  const char *XOR[] = {"PXOR", "VPXOR", "VXORPD", "XOR",
                                          "XORPD", "XORPS"};

  types["ADD"] = vector<string>(ADD, end(ADD));
  types["FADD"] = vector<string>(FADD, end(FADD));
  types["SUB"] = vector<string>(SUB, end(SUB));
  types["FSUB"] = vector<string>(FSUB, end(FSUB));
  types["MUL"] = vector<string>(MUL, end(MUL));
  types["FMUL"] = vector<string>(FMUL, end(FMUL));
  types["DIV"] = vector<string>(DIV, end(DIV));
  types["FDIV"] = vector<string>(FDIV, end(FDIV));
  types["AND"] = vector<string>(AND, end(AND));
  types["OR"] = vector<string>(OR, end(OR));
  types["CMP"] = vector<string>(CMP, end(CMP));
  types["FCMP"] = vector<string>(FCMP, end(FCMP));
  types["SHL"] = vector<string>(SHL, end(SHL));
  types["ASHR"] = vector<string>(ASHR, end(ASHR));
  types["LSHR"] = vector<string>(LSHR, end(LSHR));
  types["CALL"] = vector<string>(CALL, end(CALL));
  types["XOR"] = vector<string>(XOR, end(XOR));

  init_if_not_exists(new string("ADD"));
  init_if_not_exists(new string("FADD"));
  init_if_not_exists(new string("SUB"));
  init_if_not_exists(new string("FSUB"));
  init_if_not_exists(new string("MUL"));
  init_if_not_exists(new string("FMUL"));
  init_if_not_exists(new string("DIV"));
  init_if_not_exists(new string("FDIV"));
  init_if_not_exists(new string("AND"));
  init_if_not_exists(new string("OR"));
  init_if_not_exists(new string("CMP"));
  init_if_not_exists(new string("FCMP"));
  init_if_not_exists(new string("SHL"));
  init_if_not_exists(new string("ASHR"));
  init_if_not_exists(new string("LSHR"));
  init_if_not_exists(new string("CALL"));
  init_if_not_exists(new string("XOR" ) );

}

std::string check_mnemonic(const string &m){

  for (map<string, vector<string> >::iterator it = types.begin(); it != types.end(); it++){
    if (std::find(it->second.begin(), it->second.end(), m) != it->second.end()){
      return it->first;
    }
  }
  
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

  for (map<string, vector<string> >::iterator it = types.begin(); it != types.end(); it++){
    out << it->first;
  }
  out << "\n";

  for (map<string, vector<string> >::iterator it = types.begin(); it != types.end(); it++){
    out << bef[it->first] << "," << ma[it->first] << "," << en[it->first];
  }
  out << "\n";
  
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
  
  init();

  // filter.Activate();

  PIN_InitSymbols();
  IMG_AddInstrumentFunction(Image, 0);

  TRACE_AddInstrumentFunction(Trace, 0);

  PIN_AddFiniFunction(Fini, NULL);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
