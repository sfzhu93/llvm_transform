// RUN: clang-16 -fpass-plugin=/home/jdoe/llvm_transform/build/libMyPeepholeAdd.so -S -emit-llvm %s -o - | FileCheck-16 %s
volatile int foo(int x) {
    return x;
}

// CHECK-LABEL: main()
int main() {
    int a = 1;
    a = a + 0;
    // CHECK-NOT: add
    int b = 1;
    b = 0 + b;
    // CHECK-NOT: add
    foo(a);
    foo(b);
    return 0;
}