#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <utility>
#include <unordered_map>
#include <stdint.h>
#include <limits>
#include <random>

typedef int VertexId;

struct Partition {
    int PtId; // Partition ID
    std::set<VertexId> setVertex; // Mirror vertices
    std::set<VertexId> setMaster; // Master vertices
    std::set<std::pair<VertexId, VertexId>> setEdge; // Edges

    Partition(int id) : PtId(id) {}
};

void printPartition(const Partition &p){
    std::cout << "Partition " << p.PtId << '\n';
    std::cout << "Number of master vertices: " << p.setMaster.size() << '\n';
    std::cout << "Number of total vertices: " << (p.setVertex.size() + p.setMaster.size()) << '\n';
    std::cout << "Number of edges in this partition: " << p.setEdge.size() << '\n';
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s [input-binary-edge-file] [p]\n", argv[0]);
        exit(-1);
    }

    int p = atoi(argv[2]);
    if (p <= 0) {
        fprintf(stderr, "p must be > 0\n");
        exit(-1);
    }

    std::vector<Partition> Pts;
    Pts.reserve(p);
    for (int i = 0; i < p; ++i) Pts.emplace_back(i);

    // Read all edges into memory first (so we can greedily assign with balancing)
    std::vector<std::pair<VertexId, VertexId>> edges;
    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        perror("fopen input");
        return -1;
    }

    while (true) {
        VertexId src, dst;
        if (fread(&src, sizeof(src), 1, fin) == 0) break;
        if (fread(&dst, sizeof(dst), 1, fin) == 0) break;
        edges.emplace_back(src, dst);
    }
    fclose(fin);

    std::unordered_map<VertexId, int> vertexMaster; // vertex -> master partition id
    
    std::mt19937 rng(std::random_device{}());

    for (const auto &edge : edges) {
        VertexId u = edge.first;
        VertexId v = edge.second;

        int bestP = -1;
        int bestCost = std::numeric_limits<int>::max();
        size_t bestLoad = std::numeric_limits<size_t>::max();
        std::vector<int> tiedPartitions;

        for (int i = 0; i < p; ++i) {
            bool uPresent = (Pts[i].setMaster.count(u) || Pts[i].setVertex.count(u));
            bool vPresent = (Pts[i].setMaster.count(v) || Pts[i].setVertex.count(v));

            int cost = (!uPresent ? 1 : 0) + (!vPresent ? 1 : 0);
            size_t load = Pts[i].setEdge.size();

            if (cost < bestCost) {
                bestCost = cost;
                bestLoad = load;
                bestP = i;
                tiedPartitions.clear();
                tiedPartitions.push_back(i);
            } else if (cost == bestCost && load < bestLoad) {
                bestLoad = load;
                bestP = i;
                tiedPartitions.clear();
                tiedPartitions.push_back(i);
            } else if (cost == bestCost && load == bestLoad) {
                tiedPartitions.push_back(i);
            }
        }

        // If there are ties, pick randomly
        if (!tiedPartitions.empty()) {
            std::uniform_int_distribution<> dis(0, tiedPartitions.size() - 1);
            bestP = tiedPartitions[dis(rng)];
        }

        Pts[bestP].setEdge.insert(edge);

        // handle u
        auto itU = vertexMaster.find(u);
        if (itU == vertexMaster.end()) {
            vertexMaster[u] = bestP;
            Pts[bestP].setMaster.insert(u);
        } else {
            if (itU->second != bestP) {
                Pts[bestP].setVertex.insert(u);
            }
        }

        // handle v
        auto itV = vertexMaster.find(v);
        if (itV == vertexMaster.end()) {
            vertexMaster[v] = bestP;
            Pts[bestP].setMaster.insert(v);
        } else {
            if (itV->second != bestP) {
                Pts[bestP].setVertex.insert(v);
            }
        }
    }

    // Print partition stats
    for (int i = 0; i < p; ++i) {
        printPartition(Pts[i]);
    }

    // Write detailed output
    std::ofstream fout("partitions_heuristic.txt");
    if (!fout) {
        std::cerr << "Error: cannot open output file.\n";
        return -1;
    }
    for (int i = 0; i < p; ++i) {
        fout << "Partition " << i << "\n";
        fout << "Master vertex\n";
        for (const auto &v : Pts[i].setMaster) fout << v << "\n";

        fout << "Mirror vertex\n";
        for (const auto &v : Pts[i].setVertex) fout << v << "\n";

        fout << "Edges: \n";
        for (const auto &e : Pts[i].setEdge) fout << e.first << ' ' << e.second << "\n";

        fout << "\n";
    }
    fout.close();

    return 0;
}
