/*
 *  simpleio_ssf
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mpi.h>
#include <fcntl.h>
#include "simpleio_ssf.h"

/*
 * posix_call_fwrite_int
 * @my_rank int thread's MPI rank
 * @size size_t amount of bytes to write
 * @filename char* path to the file to write
 * @world_size int MPI's world size (total number of threads)
 * @return size_t total number of bytes written
 */
size_t posix_call_fwrite_int(int my_rank, size_t size, char *filename, int world_size) {
    FILE *fp;
    int *values;
    long offset;
    size_t data_written, total;
    unsigned int nelements;

    nelements = IOSIZE / sizeof(int);
    values = malloc(sizeof(int) * nelements);
    memset(values, 42, IOSIZE);
    data_written = 0;
    total = 0;
    offset = my_rank * size;

    fp = fopen(filename, "w");
    fseek(fp, offset, SEEK_SET);

    while(total < size) {
        data_written = fwrite(values, sizeof(int), nelements, fp);
        total = total + data_written * sizeof(int);
        fsync(fileno(fp));
    }

    ioctl(fileno(fp), BLKFLSBUF);
    fclose(fp);
    free(values);

    return total;
}

/*
 * posix_call_fread_int
 * @my_rank int thread's MPI rank
 * @size size_t amount of bytes to read
 * @filename char* path to the file to read
 * @world_size int MPI's world size (total number of threads)
 * @return size_t total number of bytes read
 */
size_t posix_call_fread_int(int my_rank, size_t size, char *filename, int world_size) {
   FILE *fp;
   int *values;
   long offset;
   size_t data_read, total;
   unsigned int nelements;

   nelements = IOSIZE / sizeof(int);
   values = malloc(sizeof(int) * nelements);
   memset(values, 0, IOSIZE);
   data_read = 0;
   total = 0;
   offset = ((my_rank + 1) % world_size) * size;

   fp = fopen(filename, "r");
   fseek(fp, offset, SEEK_SET);

   while(total < size) {
       data_read = fread(values, sizeof(int), nelements, fp);
       total = total + data_read * sizeof(int);
   }

   fclose(fp);
   free(values);

   return total;
}

/*
 *  Main part of the code: call the various read / write function
 */
int main(int argc, char **argv) {
    int rank, size, fd;
    size_t filesize, iosize, data_written, data_read;
    char filename[128];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Program parameters */
    filesize = FILESIZE;
    iosize = IOSIZE;
    sprintf(filename, "posix_ssf");

    if(rank == MASTER) {
        fd = open(filename, O_CREAT|O_WRONLY, S_IWUSR|S_IRUSR);
        posix_fallocate(fd, 0, FILESIZE);
        fsync(fd);
        close(fd);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* POSIX WRITE */
    printf("\nrank: %d, performing POSIX write operations\n", rank);
    data_written = posix_call_fwrite_int(rank, filesize/size, filename, size);
    printf("rank: %d, %u bytes written\n", rank, (unsigned int)data_written);

    MPI_Barrier(MPI_COMM_WORLD);

    /* POSIX READ */
    printf("\nrank: %d, performing POSIX read operations\n", rank);
    data_read = posix_call_fread_int(rank, filesize/size, filename, size);
    printf("rank: %d, %u bytes read\n", rank, (unsigned int)data_read);

    MPI_Finalize();
    return 0;
}
