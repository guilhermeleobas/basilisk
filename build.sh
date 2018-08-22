export LLVM_DIR=$HOME/Programs/llvm61/build/lib/cmake

make -C Collect

rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$HOME/Programs/llvm61/build ..
make

cp $(pwd)/Instrument/DCCBasilisk.* $LLVM_DIR/../

# cd Instrument
# ./configure
# make install
#
# cd ..

# make install && \
# $LLVM_PATH/clang -S -emit-llvm ../test/t2.c -o t1.bc && \
# $LLVM_PATH/opt  -load DCC888.dylib -Instrument -debug-only=Instrument t1.bc -S -o t1.rbc && \
# $LLVM_PATH/opt t1.rbc -o t1.o && \
# $LLVM_PATH/clang ../Collect/collect.o t1.o -O2 -o t1.exe
