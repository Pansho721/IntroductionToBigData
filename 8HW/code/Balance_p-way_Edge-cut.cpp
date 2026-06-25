#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <utility>


typedef int VertexId;

struct Partition {
    int PtId; // Partition ID
    std::set<VertexId> setVertex; // Set of vertices
    std::set<VertexId> setMaster; // Set of masters
    std::set<std::pair<VertexId, VertexId>> setEdge; // Set of edges represented as pairs of vertices

    Partition(int id,
              const std::set<VertexId>& vertices,
              const std::set<VertexId>& master,
              const std::set<std::pair<VertexId, VertexId>>& edgesSet)
        : PtId(id),
          setVertex(vertices), setMaster(master), setEdge(edgesSet) {}
};

int hashInt(int x) {
    return x - 1;
}

int master(int x, int p) {
    return (unsigned) hashInt(x) % p;
}

void printPartition(Partition p){
    std::cout << "Partition " << p.PtId << '\n';
    std::cout << "Number of master vertices: " << p.setMaster.size() << '\n';
    std::cout << "Number of total vertices: " << p.setVertex.size()+p.setMaster.size() << '\n';
    std::cout << "Number of replicated edges in this partition: " << p.setVertex.size() << '\n';
    std::cout << "Number of edges in this partition: " << p.setEdge.size() << '\n';
    return;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: [executable] [input]\n");
        exit(-1);
    }

    int p = atoi(argv[2]);
    std::vector<Partition> Pts;
    for (int i=0; i<p ; i++){
        std::set<VertexId> setVertex;
        std::set<VertexId> setMaster;
        std::set<std::pair<VertexId, VertexId>> setEdge;
        Pts.emplace_back(i, setVertex, setMaster, setEdge);
    }


    FILE *fin = fopen(argv[1], "rb");
    while (true) {
        VertexId src, dst;
        if (fread(&src, sizeof(src), 1, fin) == 0) break;
        if (fread(&dst, sizeof(dst), 1, fin) == 0) break;
//        printf("edge: (%d %d)\n", src, dst);
        auto edge = std::make_pair(src,dst);
        int msrc = master(src,p);
        int mdst = master(dst,p);
        if (msrc == mdst){
            Pts[msrc].setMaster.insert(src);
            Pts[msrc].setMaster.insert(dst);
            Pts[msrc].setEdge.insert(edge);
        } else {
            //node source
            Pts[msrc].setMaster.insert(src);
            Pts[msrc].setVertex.insert(dst);
            Pts[msrc].setEdge.insert(edge);
            //node target
            Pts[mdst].setMaster.insert(dst);
            Pts[mdst].setVertex.insert(src);
            Pts[mdst].setEdge.insert(edge);
        }

    }
    fclose(fin);

    for (int i=0; i<p; i++){
        printPartition(Pts[i]);
    }

    std::ofstream fout("partitions_edge_output.txt");
    if (!fout) {
        std::cerr << "Error: cannot open output file.\n";
        return -1;
    }
    for (int i = 0; i < p; i++) {
        fout << "Partition " << i << "\n";
        fout << "Masters: \n";
        for (const auto &v : Pts[i].setMaster) {
            fout << v  << "\n";
        }
        fout << "Mirrors:\n";
        for (const auto &v : Pts[i].setVertex) {
            fout << v  << "\n";
        }

        fout << "\n"; // blank line between partitions
    }
     for (int i = 0; i < p; i++) {
        fout << "Edges: \n";
        fout << "Partition " << i << "\n";

        for (const auto &e : Pts[i].setEdge){
            fout << e.first << ' ' << e.second  << "\n";
        }

        fout << "\n"; // blank line between partitions
    }


    fout.close();


    return 0;
}
