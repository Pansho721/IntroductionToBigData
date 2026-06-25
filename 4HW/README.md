# Homework 4: GFS

Introduction to Big Data Systems course 

**Due:  October 20, 2025** 23:59 China time. Late submission results in lower (or even no) scores.

For questions or concerns, contact TA (Yunyi Chen) by WeChat. Or send an email to cyy23@mails.tsinghua.edu.cn if you could not use WeChat. 

## Overview

Read the GFS paper and answer the following questions. 

Submit a PDF report to Tsinghua web learning, or by email if you can’t access web learning.



## Part 1

The master stores three major types of metadata: the file and chunk namespaces, the mapping from files to chunks, and the locations of each chunk’s replicas. While the first two type of data are persisted by master, the locations of each chunk are not persisted in the master side.

### Q1

How does the master node get the locations of each chunks at startup?

### Q2 

What is the benefit of this approach comparing with the approach that the master persists this information?



## Part 2

Assume in a cluster of GFS of 1000 servers. Each server has 10 disks with 10TB storage capacity and 100MB/s I/O bandwidth for each disk. The ethernet that connects servers has bandwidth of 1Gbps.

### Q1
Assume one server in the cluster permanently fails. The system needs to recreate all lost replicas of the chunks previously stored on this server.
What is the minimum time required to complete this recovery,
assuming ideal network topology and full parallelism?

### Q2

For quality of service, usually the recovery traffic is throttled. If the bandwidth used for recovery is 100Mbps per machine, what is the roughly time required to recover a failure server? 

### Q3
Assume each server fails independently and has a 10,000-hour MTBF,
(a) how many server failures is likely to have in a year in this cluster, and
(b) what is the average time between any two server failures in the cluster (i.e., cluster-level MTBF)?

### Q4 

Comparing the time you got from Q2 and Q3, what is the implication number of replicas that used in GFS?