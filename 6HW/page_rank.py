import sys
from pyspark import SparkConf, SparkContext
import time

#i in V, [PR(I)/L(I), ...], assuming L = 1 for each leaving edge
def PRL(pages,rank):
    n = len(pages)
    for page in pages:
        yield (page,rank/n)

if __name__ == '__main__':

    # Create Spark context.
    conf = SparkConf()
    sc = SparkContext(conf=conf)
    lines = sc.textFile(sys.argv[1])

    first = time.time()

    links = lines.map(lambda line: tuple(line.split())).distinct().groupByKey().cache()

    n = links.count()
    ranks = links.mapValues(lambda _: 1.0/n)
    ITER = 500
    tol = 1e-6
    for i in range(ITER):
        #[key, [[url1, url2], rank]]
        new_ranks = links.join(ranks).flatMap(lambda page: PRL(page[1][0],page[1][1])).reduceByKey(lambda x,y: x+y).mapValues(lambda rank: 0.2/n + 0.8 * rank)
        diff = ranks.join(new_ranks).map(lambda x: abs(x[1][0] - x[1][1])).sum()
        if diff < tol:
            print(f"Converged after {i+1} iterations (diff = {diff:.8e})")
            ranks = new_ranks
            break
        ranks = new_ranks

    highest = ranks.sortBy(lambda x: -x[1]).take(5)
    print("5 highest:", highest)

    last = time.time()

    print("Total program time: %.2f seconds" % (last - first))
    sc.stop()

