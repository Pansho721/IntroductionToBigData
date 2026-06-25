/*
Copyright (c) 2014-2015 Xiaowei Zhu, Tsinghua University

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "core/graph.hpp"

int main(int argc, char ** argv) {
    if (argc<3) {
        fprintf(stderr, "usage: k-cores [path] [int k] [memory budget in GB]\n");
        exit(-1);
    }
    std::string path = argv[1];
    int k = atoi(argv[2]);
    long memory_bytes = (argc>=4)?atol(argv[3])*1024l*1024l*1024l:8l*1024l*1024l*1024l;
    
    Graph graph(path);
    graph.set_memory_bytes(memory_bytes);
        
    BigVector<VertexId> active(graph.path+"/active", graph.vertices);
    BigVector<VertexId> degree(graph.path+"/degree", graph.vertices);
    BigVector<VertexId> to_remove(graph.path+"/to_remove", graph.vertices);
   
    graph.set_vertex_data_bytes( graph.vertices * sizeof(VertexId) );
        
    active.fill(1);  // 1 means active
    
    // compute out-degree (source degree)
    degree.fill(0);
    graph.stream_edges<VertexId>(
        [&](Edge &e){
            write_add(&degree[e.source], 1);
            return 0;
        }, nullptr, 0, 0
    );
                            
    double start_time = get_time();
        
    // Iteratively remove vertices
    bool changed = true;
    int iteration = 0;
    
    while (changed) {
        changed = false;
        iteration++;
        to_remove.fill(0);
        
        graph.stream_vertices<VertexId>(
            [&](VertexId v){
                if (active[v] == 1 && degree[v] < k) {
                    to_remove[v] = 1;
                    changed = true;
                }
                return 0;
            }, nullptr, 0, 0
        );
        
        if (!changed) break;
        
        // For each edge, if source is being removed, decrease target's degree
        graph.stream_edges<VertexId>(
            [&](Edge &e){
                if (to_remove[e.source] == 1 && active[e.target] == 1) {
                    write_add(&degree[e.target], (VertexId)-1);
                }
                return 0;
            }, nullptr, 0, 0
        );
        
        // Mark removed vertices as inactive
        graph.stream_vertices<VertexId>(
            [&](VertexId v){
                if (to_remove[v] == 1) {
                    active[v] = 0;
                }
                return 0;
            }, nullptr, 0, 0
        );
        
        printf("Iteration %d completed\n", iteration);
    }
    
    double end_time = get_time();
    printf("k-cores calculated on coreId in %.2f seconds.\n",  end_time - start_time);
    
    // Write k-core vertices to file
    FILE* fout = fopen((graph.path+"/coreId").c_str(), "w");
    if (fout == NULL) {
        fprintf(stderr, "Cannot open file %s for writing\n", (graph.path+"/coreId").c_str());
        exit(-1);
    }
    
    VertexId core_count = 0;
    for (VertexId v = 0; v < graph.vertices; v++) {
        if (active[v] == 1) {
            fprintf(fout, "%u\n", v);
            core_count++;
        }
    }
    fclose(fout);
    
    printf("Number of vertices in %d-core: %u\n", k, core_count);
    
    return 0;
    }