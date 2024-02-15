#include <stdio.h>

int main() {
    int x = 1;

    int y = x;
    if (1 > 2) {
        y = x;
    }
    int c = y;
    while (1 > 2) {
        c = y;
    }
    int d = c;
    printf("%d\n", d);
}