PLUGIN=/home/jdoe/llvm_transform/build/libMbaAdd.so
ninja -C /home/jdoe/llvm_transform/build
# opt-16 -load-pass-plugin=$PLUGIN -p=mba-add hello.ll -S -o hello_ob.ll

clang-16 -fpass-plugin=$PLUGIN -S -emit-llvm test/hello.c -o -
clang-16 -fpass-plugin=$PLUGIN test/hello.c -o test/hello