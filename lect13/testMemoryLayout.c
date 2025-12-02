#include <stdio.h>
#include <stdlib.h>

int glob = 1;
int main() {
        static int stat = 2;
        int stack;
        char *heap = (char *)malloc(100);
        printf("Stack \t= %p\n", &stack);
        printf("Heap \t= %p\n", heap);
        printf("Global \t= %p\n", &glob);
        printf("Static \t= %p\n", &stat);
        printf("main \t= %p\n", main);
        return 0;
}

