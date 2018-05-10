##======- Makefile --------------------------------------*- Makefile -*-======##
##===----------------------------------------------------------------------===##
PROJECT_NAME = Basilisk
LIBRARYNAME = DCCBasilisk
LOADABLE_MODULE = 1
USEDLIBS =
LEVEL = .
LLVM_SRC_ROOT = /home/guilhermel/Programs/llvm38
LLVM_OBJ_ROOT = /home/guilhermel/Programs/llvm38/build
PROJ_SRC_ROOT = .
PROJ_OBJ_ROOT = .
PROJ_INSTALL_ROOT = /home/guilhermel/Programs/llvm38/build/Debug+Asserts/lib/..
include $(LLVM_OBJ_ROOT)/Makefile.config
CXXFLAGS += -std=c++0x -Wno-deprecated-declarations -fexceptions -Wall -Wextra
include $(LLVM_SRC_ROOT)/Makefile.rules
