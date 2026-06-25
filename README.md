# Introduction to Big Data Systems — Homework Summary

## Homework 1
Report of HW1.pdf

## Homework 2
Technologies used: C++, OpenMP, graph algorithms
In this assignment, the task is to parallelize PageRank on a multi-core CPU using OpenMP. The implementation works on binary graph input formats and is designed to run on the course server, with correctness and performance evaluated on the `com-orkut_117m.graph` dataset.

## Homework 3
Technologies used: C++, MPI
This homework implements a custom reduction algorithm using MPI point-to-point communication. It focuses on correctness for NP=2 and NP=4 cases, handling large integer arrays and comparing against standard `MPI_Reduce` behavior.

## Homework 4
Report of Google File System paper

## Homework 5
Technologies used: Java, Hadoop MapReduce
The assignment builds a MapReduce program to compute vertex out-degrees in a graph and optionally find the top out-degree vertices. It uses Hadoop Java APIs, mapper/reducer logic, and scripts for running on HDFS.

## Homework 6
Technologies used: Python, Apache Spark
This homework contains Spark implementations for word count and PageRank. The Python programs read input from HDFS, compute top-10 frequent words and iterative PageRank scores, and report timing for Spark RDD operations.

## Homework 7
Report of The Rise of Worse is Better

## Homework 8
Technologies used: C++, graph partitioning
The task implements balanced p-way edge-cut and vertex-cut graph partitioning, with analysis of partition quality metrics like master vertices, replicated edges, and load balance. The assignment also covers greedy heuristic and hybrid-cut ideas.

## Homework 9
Technologies used: C++, GridGraph, graph algorithms
This homework uses the GridGraph framework to implement two large-scale graph algorithms from options such as conductance, k-cores, or PageRank-delta. It targets the LiveJournal dataset and emphasizes out-of-core graph processing.

## Homework 10
Technologies used: Python/C++, graph mining
The assignment implements frequent subgraph mining on labeled transaction graphs. It discovers patterns of size 3 with support >= 10000, matching node and edge labels and outputting frequent patterns in JSON form.

## Homework 11
Technologies used: Java/Scala/Python, Apache Spark Streaming
This homework builds a Spark Streaming Top-K application that listens to an HDFS directory, increments cumulative word counts each minute, and outputs the top 100 words for each 5-minute stream period.

## Homework 12
Technologies used: C++, Halide
The task optimizes a dilated convolution operator with Halide scheduling. It focuses on improving parallelism and data locality for a 4D convolution, benchmarking against oneDNN while maintaining correctness on random inputs.
