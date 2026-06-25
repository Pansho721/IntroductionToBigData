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
    if (argc < 3) {
        fprintf(stderr, "usage: pagerank-delta [path] [iterations] [memory GB]\n");
        return -1;
    }
    
    std::string path = argv[1];
    int iterations = atoi(argv[2]);
    long memory_bytes = (argc>=4)?atol(argv[3])*1024l*1024l*1024l:8l*1024l*1024l*1024l;

    printf("Using graph path: %s\n", path.c_str());

    Graph graph(path);
    graph.set_memory_bytes(memory_bytes);

    const float threshold = 0.15f;   // propagation threshold
    const float d = 0.85f;           // damping factor

    BigVector<VertexId> degree(graph.path + "/degree", graph.vertices);
    BigVector<float> pagerank(graph.path + "/pagerank", graph.vertices);
    BigVector<float> delta(graph.path + "/delta", graph.vertices);
    BigVector<float> next_delta(graph.path + "/next_delta", graph.vertices);
    Bitmap active(graph.vertices);
    Bitmap active_next(graph.vertices);

    graph.set_vertex_data_bytes( graph.vertices * sizeof(VertexId) );

    double t0 = get_time();

    // compute out-degree (source degree)
    degree.fill(0);
    graph.stream_edges<VertexId>(
        [&](Edge &e){
            write_add(&degree[e.source], 1);
            return 0;
        }, nullptr, 0, 0
    );

    // initialize pagerank and delta
    graph.stream_vertices<VertexId>(
        [&](VertexId v){
            pagerank[v] = 1.0f;
            delta[v] = (degree[v] > 0) ? (1.0f / degree[v]) : 0.0f;
            return 0;
        }, nullptr, 0,
        [&](std::pair<VertexId, VertexId> r){
            pagerank.load(r.first, r.second);
            delta.load(r.first, r.second);
        },
        [&](std::pair<VertexId, VertexId> r){
            pagerank.save();
            delta.save();
        }
    );

    // activate all vertices initially
    active.fill();

    // main loop
    for (int iter = 0; iter < iterations; ++iter) {
        printf("pagerank-delta iter %d\n", iter);
        next_delta.fill(0);
        active_next.clear();

        // propagate deltas across edges (only from active sources)
        graph.stream_edges<VertexId>(
            [&](Edge &e){
                if (active.get_bit(e.source)) {
                    if (degree[e.source] > 0) {
                        float contrib = delta[e.source] / (float)degree[e.source];
                        write_add(&next_delta[e.target], d * contrib);
                    }
                }
                return 0;
            }, nullptr, 0, 1,
            [&](std::pair<VertexId,VertexId> r){ },
            [&](std::pair<VertexId,VertexId> r){ }
        );

        // apply next_delta to pagerank, compute new delta and active_next
        graph.stream_vertices<float>(
            [&](VertexId v){
                if (next_delta[v] != 0.0f) {
                    pagerank[v] += next_delta[v];
                    float rel = fabsf(next_delta[v] / pagerank[v]);
                    if (rel > threshold) active_next.set_bit(v);
                }
                delta[v] = next_delta[v];
                return 0;
            }, nullptr, 0,
            [&](std::pair<VertexId,VertexId> r){
                pagerank.load(r.first, r.second);
                delta.load(r.first, r.second);
                next_delta.load(r.first, r.second);
            },
            [&](std::pair<VertexId,VertexId> r){
                pagerank.save();
                delta.save();
                next_delta.save();
            }
        );

        std::swap(active, active_next);
        size_t active_count = 0;
        for (size_t i = 0; i < (size_t) graph.vertices; i++) if (active.get_bit(i)) active_count++;
        if (active_count == 0) {
            printf("converged at iter %d\n", iter);
            break;
        }
    }

    double t1 = get_time();

    FILE* fout = fopen((graph.path+"/RankDelta").c_str(), "w");
    if (fout == NULL) {
        fprintf(stderr, "Cannot open file %s for writing\n", (graph.path+"/RankDelta").c_str());
        exit(-1);
    }
    
    for (VertexId v = 0; v < graph.vertices; v++) {
        fprintf(fout, "%f\n", pagerank[v]);
        
    }
    fclose(fout);

    printf("pagerank-delta done in %.2f seconds\n", t1 - t0);

    return 0;
}
