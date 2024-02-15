PLUGIN=/home/jdoe/llvm_transform/build/libMyPeepholeAdd.so
ninja -C /home/jdoe/llvm_transform/build
# opt-16 -load-pass-plugin=$PLUGIN -p=my-peephole-add hello.ll -S -o hello_opt.ll

clang-16 -fpass-plugin=$PLUGIN -S -emit-llvm test/hello.c -o -