#include "your_reduce.h"
#include <string.h>
void YOUR_Reduce(const int *sendbuf, int *recvbuf, int count) {
    int rank, np;
    int bufsize = count * sizeof(int);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // Temporary working buffer
    int *localbuf = (int *)malloc(bufsize);
    memcpy(localbuf, sendbuf, bufsize);

    for(int step=1;step<np;step*=2){
        if (rank % (2 * step) == 0) {
            int src = rank + step;
            if (src < np) {
                int *recv_tmp = (int *)malloc(bufsize);
                MPI_Recv(recv_tmp, count, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for (int i = 0; i < count; i++)
                    localbuf[i] += recv_tmp[i];
                free(recv_tmp);
            }
        } else {
            int dest = rank - step;
            MPI_Send(localbuf, count, MPI_INT, dest, 0, MPI_COMM_WORLD);
            free(localbuf);
            return;  // Done after sending
        }
    }

    // Rank 0 stores the result in recvbuf
    if (rank == 0)
        memcpy(recvbuf, localbuf, bufsize);

    free(localbuf);
}