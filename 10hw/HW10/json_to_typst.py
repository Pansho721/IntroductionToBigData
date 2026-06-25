#!/usr/bin/env python3
"""
Convert bdci_data.json FSM patterns to Typst format
"""

import json
import sys

def json_to_typst(json_file, output_file=None, limit=None):
    """
    Convert JSON FSM patterns to Typst list format
    
    Args:
        json_file: Path to bdci_data.json
        output_file: Optional output file path (defaults to stdout)
        limit: Optional limit on number of patterns to convert (default: all)
    """
    
    # Load JSON
    with open(json_file, 'r') as f:
        patterns = json.load(f)
    
    # Limit patterns if specified
    if limit:
        patterns = patterns[:limit]
    
    # Build Typst output
    lines = []
    
    for idx, pattern in enumerate(patterns):
        frequency = pattern['frequency']
        cycle = pattern['format']
        nodes = pattern['nodes']
        edges = pattern['edges']
        
        # Main pattern entry
        lines.append(f"+ Frequency: {frequency}")
        lines.append(f"  - Format: {cycle}")
        
        # Add vertices
        for node in nodes:
            node_id = node['node_id']
            name = node['name']
            lines.append(f"  - Vertex {node_id}")
            lines.append(f"    - Name is {name}")
        
        # Add edges
        edge_labels = ['a', 'b', 'c']
        for edge_idx, edge in enumerate(edges):
            label = edge_labels[edge_idx] if edge_idx < len(edge_labels) else str(edge_idx)
            amt = edge['amt']
            strategy = edge['strategy_name']
            buscode = edge['buscode']
            
            lines.append(f"  - Edge {label}")
            lines.append(f"    - amount of money is {amt}")
            lines.append(f"    - strategy is {strategy}")
            lines.append(f"    - business code is {buscode}")
        
        # Add blank line between patterns (except after last one)
        if idx < len(patterns) - 1:
            lines.append("")
    
    # Write output
    output_text = "\n".join(lines)
    
    if output_file:
        with open(output_file, 'w') as f:
            f.write(output_text)
        print(f"Typst output written to {output_file}")
    else:
        print(output_text)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 json_to_typst.py <json_file> [output_file] [limit]")
        print("Example: python3 json_to_typst.py bdci_data.json output.typst 10")
        sys.exit(1)
    
    json_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    limit = int(sys.argv[3]) if len(sys.argv) > 3 else None
    
    json_to_typst(json_file, output_file, limit)
