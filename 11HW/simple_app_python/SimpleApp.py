from __future__ import print_function
import sys
from pyspark import SparkContext
from pyspark.streaming import StreamingContext

sc = SparkContext(appName="Py_HDFSWordCount")
ssc = StreamingContext(sc, 60)

lines = ssc.textFileStream("hdfs://intro00:8020/user/2025403471/stream")
counts = lines.flatMap(lambda line: line.split(" ")).map(lambda x: (x, 1)).reduceByKey(lambda a, b: a+b)

def top_100(rdd):
    return sc.parallelize(
        rdd.takeOrdered(100, key=lambda x: -x[1])
    )

top_counts = counts.transform(top_100)

top_counts.pprint()
top_counts.saveAsTextFiles("hdfs://intro00:8020/user/2025403471/hw11output/part")

ssc.start()
ssc.awaitTerminationOrTimeout(600) # 10 minutes max