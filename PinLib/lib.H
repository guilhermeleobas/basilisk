#pragma once

static std::map<std::string, unsigned long long int> bef;
static std::map<std::string, unsigned long long int> ma;
static std::map<std::string, unsigned long long int> en;
static std::set<std::string> types;
static bool valid = true;
static std::string prefix = "before";

static std::string load  = "load";
static std::string store = "store";

VOID init_if_not_exists(const string *type){

  if (bef.find(*type) == bef.end())
    bef[*type] = 0;
  if (ma.find(*type) == ma.end())
    ma[*type] = 0;
  if (en.find(*type) == en.end())
    en[*type] = 0;
}

VOID mark_start(){
  valid = false;
}

VOID mark_end(){
  valid = true;
}

VOID main_start(){
  prefix = "main";
}

VOID main_end(){
  prefix = "end";
}

VOID Image(IMG img, VOID *v){

    RTN rtn = RTN_FindByName(img, "main");
    if (RTN_Valid(rtn)){
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)main_start, IARG_END);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)main_end, IARG_END);
        RTN_Close(rtn);
    }

}
