PLUGIN=/home/jdoe/llvm_transform/build/libMyConstProp.so
ninja -C /home/jdoe/llvm_transform/build
opt-16 -load-pass-plugin=$PLUGIN -p=my-cp test/cp_ex2.ll -S -o test/cp_ex2_after.ll

# clang-16 -fpass-plugin=$PLUGIN -S -emit-llvm test/hello.c -o -
# clang-16 -fpass-plugin=$PLUGIN test/hello.c -o test/hello