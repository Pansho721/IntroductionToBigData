#include "graph.h"

// Load vertices from a file
void loadVertices(const std::string& filename,
                  std::unordered_map<VertexID, Vertex>& vertexMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;

        Vertex v;

        std::getline(iss, token, ',');
        v.id = std::stoll(token);

        std::getline(iss, token, ',');
        v.name = token;

        std::getline(iss, token, ',');
        v.timestamp = std::stoll(token);

        std::getline(iss, token, ',');
        v.black = std::stoi(token);

        vertexMap[v.id] = v;
    }

    file.close();
}

// Load edges from a file
void loadEdges(const std::string& filename, Graph& g) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string token;

        LightEdge e;

        std::getline(iss, token, ',');
        e.source = std::stoll(token);

        std::getline(iss, token, ',');
        e.target = std::stoll(token);

        std::getline(iss, token, ','); // skip timestamp

        std::getline(iss, token, ',');
        e.amt = std::stod(token);

        std::getline(iss, token, ',');
        e.strategy_name = token;

        std::getline(iss, token, ','); // skip trade_no

        std::getline(iss, token, ',');
        e.buscode = token;

        int idx = g.edges.size();
        g.edges.push_back(e);

        g.outAdj[e.source].push_back(idx);
        g.inAdj[e.target].push_back(idx);
    }

    file.close();
}

// Load entire graph
Graph loadGraph(const std::string& dataPath) {
    Graph g;

    std::cout << "Loading account vertices..." << std::endl;
    loadVertices(dataPath + "/account", g.vertexMap);

    std::cout << "Loading card vertices..." << std::endl;
    loadVertices(dataPath + "/card", g.vertexMap);

    std::cout << "Loading account_to_account edges..." << std::endl;
    loadEdges(dataPath + "/account_to_account", g);

    std::cout << "Loading account_to_card edges..." << std::endl;
    loadEdges(dataPath + "/account_to_card", g);

    std::cout << "Graph loaded: "
              << g.vertexMap.size() << " vertices, "
              << g.edges.size() << " edges" << std::endl;

    return g;
}