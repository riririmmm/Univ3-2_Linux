#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int a = 1;
    char b = 'b';
    float c = 3.3f;
    int* p;
    a = 3;
    *p = 3; // error
    return 0;
}

