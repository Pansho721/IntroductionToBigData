#!/bin/bash



### ========= USER SETTINGS =========
# List of graphs (only names; ".graph" will be added automatically)
GRAPHS=("small-5" "roadNet-PA" "synthesized-1b" "twitter-2010")

# Partition sizes to test
PARTS=(2 3 4 8)

# User-defined timeout (in seconds)
TIMEOUT=$1

if [ -z "$TIMEOUT" ]; then
    echo "Error: please provide timeout in seconds."
    echo "Usage: ./start.sh <timeout>"
    exit 1
fi
### =================================


rm -fr Result Partition
mkdir Result Partition


########################################
# Compile all executables only once
########################################
g++ Balanced_P-way_Edge-cut.cpp -o EdgeCut
g++ Balanced_P-way_Vertex-cut.cpp -o BalVerCut
g++ Greedy_Heuristic_Vertex-cut.cpp -o HeuVerCut
g++ Balanced_P-way_Hybrid-cut.cpp -o BalHybCut


########################################
# Main loop for each graph
########################################

for G in "${GRAPHS[@]}"; do
    GRAPH_PATH="/data/hw8_data/${G}.graph"

    echo "=== Processing graph: $G ==="
    
    #### ---------- Balanced Edge-Cut ---------- ####
    OUT="Result/EdgeCut_${G}.txt"
    touch "$OUT"
    echo "Graph: $G" >> "$OUT"
    echo "===========================" >> "$OUT"

    echo "Balanced P-way Edge-cut"
    for P in "${PARTS[@]}"; do
        echo "Graph: ${G}    Number of Machines: ${P}"
        echo "${P} Partitions" >> "$OUT"
        echo "===========================" >> "$OUT"

        timeout $TIMEOUT ./EdgeCut "$GRAPH_PATH" "$P" >> "$OUT"
        
        # File created by this algorithm
        if [ -f partitions_edge_output.txt ]; then
            mv partitions_edge_output.txt "Partition/BEC_${G}_${P}.txt"
        fi
    done


    #### ---------- Balanced Vertex-Cut ---------- ####
    OUT="Result/BalVerCut_${G}.txt"
    touch "$OUT"
    echo "Graph: $G" >> "$OUT"
    echo "===========================" >> "$OUT"
    echo "Balanced P-way Vertex-cut"

    for P in "${PARTS[@]}"; do
        echo "Graph: ${G}    Number of Machines: ${P}"
        echo "${P} Partitions" >> "$OUT"
        echo "===========================" >> "$OUT"

        timeout $TIMEOUT ./BalVerCut "$GRAPH_PATH" "$P" >> "$OUT"

        if [ -f partitions_vertex_output.txt ]; then
            mv partitions_vertex_output.txt "Partition/BVC_${G}_${P}.txt"
        fi
    done


    #### ---------- Heuristic Vertex-Cut ---------- ####
    OUT="Result/HeuVerCut_${G}.txt"
    touch "$OUT"
    echo "Graph: $G" >> "$OUT"
    echo "===========================" >> "$OUT"
    echo "Greedy Heuristic Vertex-cut"

    for P in "${PARTS[@]}"; do
        echo "Graph: ${G}    Number of Machines: ${P}"
        echo "${P} Partitions" >> "$OUT"
        echo "===========================" >> "$OUT"

        timeout $TIMEOUT ./HeuVerCut "$GRAPH_PATH" "$P" >> "$OUT"

        if [ -f partitions_heuristic.txt ]; then
            mv partitions_heuristic.txt "Partition/HVC_${G}_${P}.txt"
        fi
    done


    #### ---------- Balanced Hybrid-Cut ---------- ####
    OUT="Result/BalHybCut_${G}.txt"
    touch "$OUT"
    echo "Graph: $G" >> "$OUT"
    echo "===========================" >> "$OUT"
    echo "Balanced P-way Hybrid-cut"

    for P in "${PARTS[@]}"; do
        echo "Graph: ${G}    Number of Machines: ${P}"
        echo "${P} Partitions" >> "$OUT"
        echo "===========================" >> "$OUT"

        timeout $TIMEOUT ./BalHybCut "$GRAPH_PATH" "$P" >> "$OUT"

        if [ -f partitions_hybrid_output.txt ]; then
            mv partitions_hybrid_output.txt "Partition/BHC_${G}_${P}.txt"
        fi
    done

done
