#include <stdio.h>
#include <stdlib.h>

typedef int VertexId;

int main(int argc, char ** argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: [executable] [input] [output]\n");
    exit(-1);
  }
  FILE * fin = fopen(argv[1], "rb");
  FILE * fout = fopen(argv[2], "wb");
  while (true) {
    VertexId src, dst;
    if (fread(&src, sizeof(src), 1, fin)==0) break;
    if (fread(&dst, sizeof(dst), 1, fin)==0) break;
    fwrite(&src, sizeof(src), 1, fout);
    fwrite(&dst, sizeof(dst), 1, fout);
    fwrite(&dst, sizeof(dst), 1, fout);
    fwrite(&src, sizeof(src), 1, fout);
  }
  fclose(fin);
  fclose(fout);
}
