
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef int VertexId;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: [executable] [input]\n");
        exit(-1);
    }
    FILE *fin = fopen(argv[1], "rb");
    while (true) {
        VertexId src, dst;
        if (fread(&src, sizeof(src), 1, fin) == 0) break;
        if (fread(&dst, sizeof(dst), 1, fin) == 0) break;
        printf("edge: (%d %d)\n", src, dst);
    }
    fclose(fin);
    return 0;
}