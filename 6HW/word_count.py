import re
import sys
from pyspark import SparkConf, SparkContext
import time


if __name__ == '__main__':
    conf = SparkConf()
    sc = SparkContext(conf=conf)
    lines = sc.textFile(sys.argv[1])

    first = time.time()

    words = lines.flatMap(lambda line: re.split(r'[^\w]+', line))
    word_map = words.map(lambda w: (w, 1))
    word_count = word_map.reduceByKey(lambda a, b: a + b)
    word_count = word_count.sortBy(lambda x: -x[1])
    top = word_count.take(10)
    for word, count in top:
        print(f"{word}: {count}")
    
    last = time.time()

    print("Total program time: %.2f seconds" % (last - first))
    sc.stop()
