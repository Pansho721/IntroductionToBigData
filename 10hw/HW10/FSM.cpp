#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <omp.h>

#include "graph.h"

typedef int64_t VertexID;

// Global configuration parameters
int HIGHEST_DEGREE = 500000;   // Number of highest-degree vertices to use
int MAX_PATTERNS =   500000;   // Keep more patterns in memory

//Given a Sub-graph with 3 edge gives its canonical form string
std::string getCanonicalForm(
    const Graph& g,
    VertexID u, VertexID v, VertexID w, VertexID x,
    const LightEdge& e1,
    const LightEdge& e2,
    const LightEdge& e3
) {
    // Collect input vertex IDs
    std::vector<VertexID> vids = {u, v, w, x};
    std::vector<std::string> labels(4);
    for (int i = 0; i < 4; i++) {
        labels[i] = g.vertexMap.at(vids[i]).name;  // or appropriate access
    }

    char cycle = '2';
    if (u == x) {
        cycle = '1';
    } else {
        if (u == w and v == x){
            cycle = '4';
        } else {
            if (u == w or v == x) {
                cycle = '3';
            } else {cycle = '2';}
        }

    }

    // Prepare vector of vertex info: (label, originalIndex)
    struct VInfo {
        std::string label;
        int origIdx;
        int canonIdx;  // assigned canonical index
    };
    std::vector<VInfo> vinfo(4);

    // If cycle: assign same canonical index to same vid; else trivial
    if (cycle != '2') {
        std::map<VertexID,int> vid2canon;
        int nextCanon = 0;
        for (int i = 0; i < 4; i++) {
            VertexID vid = vids[i];
            auto it = vid2canon.find(vid);
            if (it == vid2canon.end()) {
                vid2canon[vid] = nextCanon++;
                it = vid2canon.find(vid);
            }
            vinfo[i] = { labels[i], i, it->second };
        }
    } else {
        for (int i = 0; i < 4; i++) {
            vinfo[i] = { labels[i], i, i };
        }
    }

    // Build mapping original index to canonical index
    int canonOf[4];
    for (auto &vi : vinfo) {
        canonOf[vi.origIdx] = vi.canonIdx;
    }

    // Canonicalize edges
    struct EInfo {
        int s, t;
        std::string strategy;
        std::string buscode;
        long long amt;
    };
    std::vector<EInfo> edges;
    auto addE = [&](const LightEdge &e, int srcOrig, int tgtOrig) {
        edges.push_back({
            canonOf[srcOrig],
            canonOf[tgtOrig],
            e.strategy_name,
            e.buscode,
            (long long)std::llround(e.amt)
        });
    };

    addE(e1, 0, 1);
    addE(e2, 1, 2);
    addE(e3, 2, 3);

    // Sort edges canonically
    std::sort(edges.begin(), edges.end(),
        [](auto &A, auto &B) {
            if (A.strategy != B.strategy) return A.strategy < B.strategy;
            if (A.buscode != B.buscode)   return A.buscode < B.buscode;
            if (A.amt != B.amt)           return A.amt < B.amt;
            if (A.s != B.s)               return A.s < B.s;
            return A.t < B.t;
        });

    // Build the canonical string
    std::ostringstream oss;

    // Append Edges in canonical order
    for (size_t i = 0; i < edges.size(); i++) {
        auto &e = edges[i];
        oss << e.strategy << ":" << e.buscode << ":" << e.amt;
        if (i + 1 < edges.size()) oss << "|";
    }

    // Append cycle flag
    oss << "|" << cycle << "|";

    // Append vertices in canonical order
    for (int i = 0; i < 4; i++) {
        auto &vi = vinfo[i];
        oss << vi.canonIdx << ":" << vi.label;
        if (i < 3) oss << ",";
    }

    return oss.str();
}




// enumeratePatterns for 3 edge long subgraphs
void enumeratePatterns(const Graph& g, std::map<std::string, int>& patterns, const int SUPPORT_THRESHOLD = 10000) {
    std::cout << "Enumerating 3-edge patterns (paths and cycles, multithreaded)..." << std::endl;
    
    // Find vertices with both in and out edges, sorted by total degree
    std::vector<std::pair<VertexID, long long>> degreeList;
    
    for (auto& vEntry : g.outAdj) {
        VertexID v = vEntry.first;
        long long outDegree = vEntry.second.size();
        
        auto inIt = g.inAdj.find(v);
        long long inDegree = (inIt != g.inAdj.end()) ? inIt->second.size() : 0;
        long long totalDegree = inDegree * outDegree;
        
        if (totalDegree > 0) {
            degreeList.push_back({v, totalDegree});
        }
    }
    
    // Sort by total degree descending
    std::sort(degreeList.begin(), degreeList.end(),
        [](const std::pair<VertexID, long long>& a, const std::pair<VertexID, long long>& b) {
            return a.second > b.second;
        });
    
    int topK = std::min(HIGHEST_DEGREE, (int)degreeList.size());
    std::cout << "Using top " << topK << " high-degree vertices out of " << degreeList.size() << std::endl;
    
    // Get number of threads
    int numThreads = omp_get_max_threads();
    std::cout << "Using " << numThreads << " threads for parallel enumeration" << std::endl;
    
    // Thread-local pattern maps with memory limits
    std::vector<std::map<std::string, int>> threadPatterns(numThreads);
    long long totalEnumerated = 0;
    const int THREAD_PATTERN_LIMIT = MAX_PATTERNS / numThreads + 100;  // Per-thread limit
    
    // Parallel enumeration from top vertices
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        std::map<std::string, int>& localPatterns = threadPatterns[tid];
        long long localEnumerated = 0;
        
        #pragma omp for schedule(dynamic, 1)
        //Only get top-k vertices
        for (int v_idx = 0; v_idx < topK; v_idx++) {
            // Progress reporting (only thread 0)
            if (tid == 0 && v_idx % 100 == 0) {
                #pragma omp critical
                {
                    std::cout << "Processed " << v_idx << " / " << topK << " vertices (thread patterns: " 
                              << localPatterns.size() << std::endl;
                }
            }
            
            VertexID v = degreeList[v_idx].first;
            
            // Get incoming and outgoing edges
            auto v_in_it = g.inAdj.find(v);
            if (v_in_it == g.inAdj.end() || v_in_it->second.empty()) continue;
            const std::vector<int>& inEdges_v = v_in_it->second;
            
            auto v_out_it = g.outAdj.find(v);
            if (v_out_it == g.outAdj.end() || v_out_it->second.empty()) continue;
            const std::vector<int>& outEdges_v = v_out_it->second;
            
            // For each incoming edge (u->v)
            for (int e1_idx : inEdges_v) {
                const LightEdge& e1 = g.edges[e1_idx];
                VertexID u = e1.source;
                
                // For each outgoing edge (v->w)
                for (int e2_idx : outEdges_v) {
                    const LightEdge& e2 = g.edges[e2_idx];
                    VertexID w = e2.target;

                    // For paths: maintain ordering to avoid duplicates
                    if (e1_idx >= e2_idx) continue;
                    
                    // For each outgoing edge from w: (w->x)
                    auto w_out_it = g.outAdj.find(w);
                    if (w_out_it == g.outAdj.end() || w_out_it->second.empty()) continue;
                    
                    for (int e3_idx : w_out_it->second) {
                        const LightEdge& e3 = g.edges[e3_idx];
                        VertexID x = e3.target;

                        // For paths: maintain ordering to avoid duplicates
                        if (e2_idx >= e3_idx) continue;
                        
                        // Found a 3-edge pattern: u->v->w->x (path or cycle)
                        std::string canonical = getCanonicalForm(g, u, v, w, x, e1, e2, e3);

                        localPatterns[canonical]++;
                        localEnumerated++;
                    }
                }
            }
            
            // Trim local patterns if getting too large
            if (localPatterns.size() > THREAD_PATTERN_LIMIT * 2) {
                std::vector<std::pair<int, std::string>> freqList;
                for (auto& p : localPatterns) {
                    freqList.push_back({p.second, p.first});
                }
                std::sort(freqList.rbegin(), freqList.rend());
                
                std::map<std::string, int> trimmed;
                for (int i = 0; i < std::min((int)freqList.size(), THREAD_PATTERN_LIMIT); i++) {
                    trimmed[freqList[i].second] = freqList[i].first;
                }
                localPatterns = trimmed;
            }
        }
        
        // Accumulate total enumerated patterns
        #pragma omp atomic
        totalEnumerated += localEnumerated;
    }
    
    std::cout << "Merging results from " << numThreads << " threads..." << std::endl;
    
    // Merge thread-local patterns into global patterns
    for (int tid = 0; tid < numThreads; tid++) {
        for (auto& p : threadPatterns[tid]) {
            patterns[p.first] += p.second;
        }
    }
    
    std::cout << "Total patterns enumerated: " << totalEnumerated << std::endl;
    std::cout << "Unique patterns found: " << patterns.size() << std::endl;
    
    
    // Filter by support threshold directly
    std::map<std::string, int> filteredPatterns;
    for (auto& p : patterns) {
        if (p.second >= SUPPORT_THRESHOLD) {
            filteredPatterns[p.first] = p.second;
        }
    }
    patterns = filteredPatterns;
    
    std::cout << "Found " << patterns.size() << " patterns with support >= " << SUPPORT_THRESHOLD << std::endl;
}



// Parse canonical form to extract pattern information and store with edges
struct PatternInfo {
    std::vector<VertexID> vertexIds;
    std::vector<std::string> nodeLabels;
    std::string format;
    std::vector<std::tuple<std::string, std::string, double>> edgeLabels; // (strategy, buscode, amt)
    int frequency;
};

// Extract pattern components from canonical form
PatternInfo parseCanonical(const std::string& canonical, int frequency) {
    PatternInfo info;
    info.frequency = frequency;
    std::vector<std::string> parts;
    size_t start = 0;

    while (true) {
        size_t pos = canonical.find('|', start);
        if (pos == std::string::npos) {
            parts.push_back(canonical.substr(start));
            break;
        }
        parts.push_back(canonical.substr(start, pos - start));
        start = pos + 1;
    }

    // Must have exactly 5 parts
    // [0]=e1, [1]=e2, [2]=e3, [3]=format, [4]=nodes
    if (parts.size() != 5) {
        std::cerr << "parseCanonical ERROR: expected 5 parts, got " << parts.size()
                  << " from: " << canonical << std::endl;
        return info;
    }

    // ----------------------------------------
    // 1. Parse edges (first 3 segments)
    // ----------------------------------------
    for (int i = 0; i < 3; i++) {
        const std::string& edge = parts[i];

        std::istringstream iss(edge);
        std::string strategy, buscode, amtStr;

        std::getline(iss, strategy, ':');
        std::getline(iss, buscode, ':');
        std::getline(iss, amtStr, ':');

        double amt = std::stod(amtStr);
        info.edgeLabels.push_back({strategy, buscode, amt});
    }

    // ----------------------------------------
    // 2. Parse format
    // ----------------------------------------
    info.format = parts[3];

    // ----------------------------------------
    // 3. Parse node list
    // ----------------------------------------
    const std::string& nodePart = parts[4];
    size_t nodePos = 0;

    while (nodePos < nodePart.size()) {
        size_t commaPos = nodePart.find(',', nodePos);
        std::string nodeStr;

        if (commaPos == std::string::npos) {
            nodeStr = nodePart.substr(nodePos);
        } else {
            nodeStr = nodePart.substr(nodePos, commaPos - nodePos);
        }

        size_t colonPos = nodeStr.find(':');
        VertexID vid = std::stoll(nodeStr.substr(0, colonPos));
        std::string label = nodeStr.substr(colonPos + 1);

        info.vertexIds.push_back(vid);
        info.nodeLabels.push_back(label);

        if (commaPos == std::string::npos) break;
        nodePos = commaPos + 1;
    }

    return info;
}


// Write patterns to JSON file (sorted by frequency descending)
void writeJSON(const std::map<std::string, int>& patterns, const std::string& outputFile) {
    std::ofstream outfile(outputFile);
    if (!outfile.is_open()) {
        std::cerr << "Error opening output file: " << outputFile << std::endl;
        return;
    }
    
    // Sort patterns by frequency descending
    std::vector<std::pair<int, std::string>> sortedPatterns;
    for (auto& p : patterns) sortedPatterns.push_back({p.second, p.first});
    std::sort(sortedPatterns.rbegin(), sortedPatterns.rend());
    
    outfile << "[\n";
    bool firstPattern = true;
    
    for (auto& p : sortedPatterns) {
        if (!firstPattern) outfile << ",\n";
        firstPattern = false;
        
        int frequency = p.first;
        PatternInfo info = parseCanonical(p.second, frequency);

        outfile << "    {\n";
        outfile << "        \"frequency\": " << info.frequency << ",\n";
        outfile << "        \"format\": " << info.format << ",\n";

        // Nodes with canonical IDs
        outfile << "        \"nodes\": [\n";
        for (size_t i = 0; i < info.nodeLabels.size(); i++) {
            outfile << "            {\n";
            outfile << "                \"node_id\": " << i << ",\n";
            outfile << "                \"node_id_ref\": " << info.vertexIds[i] << ",\n";
            outfile << "                \"name\": \"" << info.nodeLabels[i] << "\"\n";
            outfile << "            }";
            if (i < info.nodeLabels.size() - 1) outfile << ",";
            outfile << "\n";
        }
        outfile << "        ],\n";
        
        // Edges using canonical vertex IDs
        outfile << "        \"edges\": [\n";
        for (size_t i = 0; i < info.edgeLabels.size(); i++) {
            const auto& e = info.edgeLabels[i];
            outfile << "            {\n";
            outfile << "                \"source_node_id\": " << i << ",\n";
            outfile << "                \"target_node_id\": " << i + 1 << ",\n";
            
            double amt = std::get<2>(e);
            if (amt == (long long)amt) {
                outfile << "                \"amt\": " << (long long)amt << ".0,\n";
            } else {
                outfile << "                \"amt\": " << amt << ",\n";
            }
            
            outfile << "                \"strategy_name\": \"" << std::get<0>(e) << "\",\n";
            outfile << "                \"buscode\": \"" << std::get<1>(e) << "\"\n";
            outfile << "            }";
            if (i < info.edgeLabels.size() - 1) outfile << ",";
            outfile << "\n";
        }
        outfile << "        ]\n";
        outfile << "    }";
    }
    
    outfile << "\n]\n";
    outfile.close();
    std::cout << "Results written to " << outputFile << " (sorted by frequency)" << std::endl;
}




int main(int argc, char **argv){
    int support;
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <data_path> [support_threshold] [Highest Degree] [Max pattern]" << std::endl;
        return 1;
    }
    std::string dataPath = argv[1];
    if (argc >= 3) {support = atoi(argv[2]);} else {support = 10000;}
    if (argc >= 4) {HIGHEST_DEGREE = atoi(argv[3]);}
    if (argc >= 5) {MAX_PATTERNS = atoi(argv[4]);}

    Graph g = loadGraph(dataPath);
    
    // Enumerate patterns
    std::map<std::string, int> patterns;
    enumeratePatterns(g, patterns, support);
    
    // Write results to JSON
    writeJSON(patterns, "bdci_data.json");
    
    printf("Graph FSM completed successfully!\n");
    
    return 0;
}