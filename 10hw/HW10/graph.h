#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

typedef int64_t VertexID;

// ------------------------------
// Vertex & Edge Structures
// ------------------------------
struct Vertex {
    VertexID id;
    std::string name;
    int64_t timestamp;
    int black;
};

struct Edge {
    VertexID source_id;
    VertexID target_id;
    int64_t timestamp;
    double amt;
    std::string strategy_name;
    std::string buscode;
    int64_t trade_no;
};

// Lightweight edge for efficient storage
struct LightEdge {
    VertexID source;
    VertexID target;
    double amt;
    std::string strategy_name;
    std::string buscode;
};

// ------------------------------
// Graph Structure
// ------------------------------
struct Graph {
    std::unordered_map<VertexID, Vertex> vertexMap;
    std::vector<LightEdge> edges;
    std::unordered_map<VertexID, std::vector<int>> outAdj;
    std::unordered_map<VertexID, std::vector<int>> inAdj;
};

// ------------------------------
// Functions
// ------------------------------
void loadVertices(const std::string& filename,
                  std::unordered_map<VertexID, Vertex>& vertexMap);

void loadEdges(const std::string& filename, Graph& g);

Graph loadGraph(const std::string& dataPath);