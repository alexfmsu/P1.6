/*
 *  simpleio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mpi.h>
#include "simpleio.h"

/*
 * posix_call_fwrite_int
 * @my_rank int thread's MPI rank
 * @filesize size_t size of the file to write in bytes
 * @filename char* path to the file to write
 * @return size_t total number of bytes written
 */
size_t posix_call_fwrite_int(int my_rank, size_t filesize, char *filename) {
    FILE *fp;
    int *value;
    size_t data_written, total;

    value = malloc(sizeof(int));
    *value = 42;
    data_written = 0;
    total = 0;

    fp = fopen(filename, "w");

    while(total < filesize) {
        data_written = fwrite(value, sizeof(int), 1, fp);
        total = total + (data_written * sizeof(int));
        fsync(fileno(fp));
    }

    ioctl(fileno(fp), BLKFLSBUF);
    fclose(fp);
    free(value);

    return total;
}

/*
 * posix_call_fread_int
 * @my_rank int thread's MPI rank
 * @filesize size_t size of the file to read in bytes
 * @filename char* path to the file to read
 * @return size_t total number of bytes read
 */
size_t posix_call_fread_int(int my_rank, size_t filesize, char *filename) {
   FILE *fp;
   int *value;
   size_t data_read, total;

   value = malloc(sizeof(int));
   data_read = 0;
   total = 0;

   fp = fopen(filename, "r");

   while(total < filesize) {
       data_read = fread(value, sizeof(int), 1, fp);
       total = total + (data_read * sizeof(int));
   }

   fclose(fp);
   free(value);

   return total;
}

/*
 * main
 */
int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    size_t filesize, iosize, data_written, data_read;
    char filename[128];

    /* Program parameters */
    filesize = FILESIZE;
    sprintf(filename, "posix_rank_%d", rank);

    /* POSIX WRITE */
    printf("\nrank: %d, performing POSIX write operations\n", rank);
    data_written = posix_call_fwrite_int(rank, filesize, filename);
    printf("rank: %d, %u bytes written\n---------------------\n", rank, (unsigned int)data_written);

    MPI_Barrier(MPI_COMM_WORLD);

    /* POSIX READ */
    printf("\nrank: %d, performing POSIX read operations\n", rank);
    data_read = posix_call_fread_int(rank, filesize, filename);
    printf("rank: %d, %u bytes read\n---------------------\n", rank, (unsigned int)data_read);

    MPI_Finalize();
    return 0;
}
