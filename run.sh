LLVM_PATH=$HOME/Programs/llvm38/build/bin

make -C Collect

cd Instrument

make install && \
$LLVM_PATH/clang -S -emit-llvm ../test/t2.c -o t1.bc && \
$LLVM_PATH/opt  -load DCC888.dylib -Instrument -debug-only=Instrument t1.bc -S -o t1.rbc && \
$LLVM_PATH/opt t1.rbc -o t1.o && \
$LLVM_PATH/clang ../Collect/collect.o t1.o -O2 -o t1.exe

pwd