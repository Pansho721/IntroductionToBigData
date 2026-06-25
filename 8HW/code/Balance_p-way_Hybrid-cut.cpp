#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <utility>

int threshold = 3;
typedef int VertexId;

struct Partition {
    int PtId; // Partition ID
    std::set<VertexId> setVertex; 
    std::set<std::pair<VertexId, VertexId>> setEdge;
    std::map<VertexId, std::set<VertexId>> degree;

    Partition(int id,
              const std::set<VertexId>& vertices,
              const std::set<std::pair<VertexId, VertexId>>& edgesSet,
              const std::map<VertexId, std::set<VertexId>>& mapDegree)
        : PtId(id),
          setVertex(vertices),
          setEdge(edgesSet),
          degree(mapDegree) {}
};

/*      HASH FUNCTION USED ON SMALL GRAPH 5
int hashInt(int x) {
    return x-1;
}*/

int hashInt(int x) {
    uint32_t v = static_cast<uint32_t>(x);

    v = (v ^ 61) ^ (v >> 16);
    v = v + (v << 3);
    v = v ^ (v >> 4);
    v = v * 0x27d4eb2d;
    v = v ^ (v >> 15);

    return static_cast<int>(v);
}

int master(int x, int p) {
    return (unsigned) hashInt(x) % p;
}

void printPartition(Partition p){
    std::cout << "Partition: " << p.PtId << '\n';
    std::cout << "Number of vertices: " << p.setVertex.size() << '\n';
    std::cout << "Number of edges: " << p.setEdge.size() << '\n';
    return;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: [executable] [input]\n");
        exit(-1);
    }

    int p = atoi(argv[2]);
    std::vector<Partition> Pts;
    for (int i = 0; i < p; i++) {
        std::set<VertexId> setVertex;
        std::set<std::pair<VertexId, VertexId>> setEdge;
        std::map<VertexId, std::set<VertexId>> degree;

        Pts.emplace_back(i, setVertex, setEdge, degree);
    }
    FILE *fin = fopen(argv[1], "rb");
    int e = 0;
    while (true) {
        VertexId src, dst;
        if (fread(&src, sizeof(src), 1, fin) == 0) break;
        if (fread(&dst, sizeof(dst), 1, fin) == 0) break;
        //printf("edge: (%d %d)\n", src, dst);

        auto edge = std::make_pair(src,dst);
        int pt = master(dst,p);
        if((Pts[pt].degree[dst].size() + 1) <= threshold){
            Pts[pt].degree[dst].insert(src);
            Pts[pt].setEdge.insert(edge);
            Pts[pt].setVertex.insert(src);
            Pts[pt].setVertex.insert(dst);
        } else {
            pt = master(src,p);
            Pts[pt].setEdge.insert(edge);
            Pts[pt].setVertex.insert(src);
            Pts[pt].setVertex.insert(dst);
        }
    }
    fclose(fin);

    for (int i=0; i<p; i++){
        printPartition(Pts[i]);
    }

    std::ofstream fout("partitions_hybrid_output.txt");
    if (!fout) {
        std::cerr << "Error: cannot open output file.\n";
        return -1;
    }
    for (int i = 0; i < p; i++) {
        fout << "Partition " << i << "\n";
        fout << "Vertex:\n";
        for (const auto &v : Pts[i].setVertex) {
            fout << v  << "\n";
        }
        fout << "Edges:\n";
        for (const auto &e : Pts[i].setEdge){
            fout << e.first << ' ' << e.second << '\n';
        }

        fout << "\n"; // blank line between partitions
    }

    fout.close();
    
    return 0;
}
