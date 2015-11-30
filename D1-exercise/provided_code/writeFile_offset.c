/* Created by S. Cozzini and G.P. Brandino for the Course P1.6 Data Management @ MHPC
 * Last Revision: November 2015
 */

#include<stdio.h>
#include<stdlib.h>
#include "mpi.h"
int main(int argc, char **argv){
	
	int i, rank, size, offset, N=16 ;
	int* buf=(int*) malloc(N*sizeof(int));
	int ad_size, rest;

	MPI_File fhw;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	for ( i=0;i<N;i++){
	  buf[i] = i ;
  	}
	
	rest = N % size;

	if(rank > rest || rest == 0){
	  ad_size = N / size;
	  offset = rank * ad_size * sizeof(int) + rest * sizeof(int);
	}
	else{
	  ad_size = N / size + 1;
	  offset = rank * ad_size * sizeof(int);
	}
	
	/* offset = rank * N * sizeof(int); */
	/* offset = rank*(N/size)*sizeof(int); */

	MPI_File_open(MPI_COMM_WORLD, "datafile", MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fhw);	
	printf("Rank: %d, Offset: %d\n", rank, offset);
	/* MPI_File_write_at(fhw, offset, buf + rank * ad_size, ad_size, MPI_INT, &status); */
	MPI_File_write_at(fhw, offset, buf + offset / sizeof(int), ad_size, MPI_INT, &status);
	/* MPI_File_write_at(fhw, offset, buf + rank * (N / size), (N/size), MPI_INT, &status); */
	/* MPI_File_write_at(fhw, offset, buf, N, MPI_INT, &status); */
	free(buf);
	MPI_File_close(&fhw);
	MPI_Finalize();

	return 0;
}
