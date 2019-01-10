rm -rf build 
mkdir build
cd build
cmake .. -DZ3_DIR="/home/amadeus/z3-4.8.3.7f5d66c3c299-x64-ubuntu-16.04/" -DLLVM_DIR="/home/amadeus/llvm-6.0.1.src/build/lib/cmake/llvm/"
make
