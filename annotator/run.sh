clang -gdwarf-2 -g -S -emit-llvm ../test/t2.c -o t1.bc
opt -load DCC888.so -annotator t1.bc -S -o t1.rbc 
llc -filetype=obj t1.rbc -o t1.o
clang -g -gdwarf-2 t1.o -o t1.exe
