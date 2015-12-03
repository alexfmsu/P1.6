/**
 *  darshan_test, 
 *  a simple benchmark which try various communication API and patterns. 
 *  The data volume of each pattern is statically known and can be
 *  used to calibrate various monitoring tools
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_ELT  100000000

typedef struct  cell {
   int    uid;
   int    bit_mask;
   double pressure;
   double x, y, z;
} Cell_t;
Cell_t my_domain[MAX_ELT];

MPI_Datatype CellType;

/**
 * Support functions
 */

void populate_domain(Cell_t *ptr_dom, int seed, int nb_cell)
{
int i;
    for (i = 0 ; i < nb_cell ; i++)
        {
        // no value set to zero to avoid divide error
        ptr_dom[i].uid = 64 + seed + i % 8;
        ptr_dom[i].bit_mask = 128 + seed + i % 64; 
        ptr_dom[i].pressure = ((double)ptr_dom[i].uid) / ptr_dom[i].bit_mask ;
        ptr_dom[i].x = 2 * ((double)ptr_dom[i].uid) / ptr_dom[i].bit_mask ;
        ptr_dom[i].y = 4 * ((double)ptr_dom[i].uid) / ptr_dom[i].bit_mask ;
        ptr_dom[i].z = 8 * ((double)ptr_dom[i].uid) / ptr_dom[i].bit_mask ;
        }
}

void dump_domain(Cell_t *ptr_dom, int nb_cell)
{
int i;
    for (i = 0 ; i < nb_cell ; i++)
        {   
        printf ("cell[%4d]: uid:%3d bit_mask:%3x pressure:%3f x:%8f y:%8f z:%8f \n",
        i, ptr_dom[i].uid, ptr_dom[i].bit_mask, ptr_dom[i].pressure, 
        ptr_dom[i].x, ptr_dom[i].y, ptr_dom[i].z);
        }   
}

void dump_int_buffer(int *ptr, int nb_elt)
{
    int i;
    for(i = 0 ; i < nb_elt ; i++)
        printf(" %d ", *ptr++); 
    printf ("\n");
}

void dump_char_buffer(char *ptr, int nb_elt)
{
    int i;
    for(i = 0 ; i < nb_elt ; i++)
        printf(" %c ", *ptr++); 
    printf ("\n");
}

/**
 *  Int read functions
 */

int* mpio_read_int_with_seek(MPI_File my_fh, int my_rank, int nb_elt, void * my_buf)
{
    MPI_Status status;
    MPI_Offset offset =  my_rank * nb_elt;
    /* set the pointer to the correct file offset */
    MPI_File_seek(my_fh, offset, MPI_SEEK_SET);
    MPI_File_read(my_fh, (int *)my_buf, nb_elt, MPI_INT, &status);
    return (int *) my_buf;
}

int* mpio_read_int_with_read_at(MPI_File my_fh, int my_rank, int nb_elt, void * my_buf)
{
    MPI_Status status;
    MPI_File_read_at(my_fh, my_rank * nb_elt, my_buf, nb_elt, MPI_INT, &status);
    return (int *) my_buf;
}

/**
 *  Char read functions
 */
int mpio_read_char_with_seek(MPI_File my_fh, int my_rank, int nb_elt, void * my_buf)
{
    MPI_Status status;
    MPI_Offset offset =  my_rank * nb_elt;
    /* set the pointer to the correct file offset */
    MPI_File_seek(my_fh, offset, MPI_SEEK_SET);
    MPI_File_read(my_fh, (char *) my_buf, nb_elt, MPI_CHAR, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

int mpio_read_char_with_read_at(MPI_File my_fh, int my_rank, int nb_elt, void * my_buf)
{
    MPI_Status status;
    MPI_File_read_at(my_fh, my_rank * nb_elt, (char *) my_buf, nb_elt, MPI_CHAR, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

/**
 *  MPI Complex data type functions
 * 
 */
int mpio_read_with_datatype(MPI_File my_fh, int my_rank, int nb_cell, void * my_mesh)
{
    MPI_Status status;
    MPI_Offset offset = my_rank * nb_cell;
    MPI_File_read_at(my_fh, offset, my_mesh, nb_cell, CellType, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

/**
 * Mpi write with complex data type
 * return 1 if OK, 0 on error
 */
int mpio_write_with_datatype(MPI_File my_fh, int my_rank, int nb_cell, void * my_mesh)
{
    MPI_Status status;
    MPI_Offset offset = my_rank * nb_cell;
    MPI_File_write_at(my_fh, offset, my_mesh, nb_cell, CellType, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

/**
 * MPI write with char data type
 * return 1 if OK, 0 on error
 */
int mpio_write_with_char(MPI_File my_fh, int my_rank, int nb_char, char * my_buf)
{
    MPI_Status status;
    MPI_Offset offset = my_rank * nb_char;
    MPI_File_write_at(my_fh, offset, my_buf, nb_char, MPI_CHAR, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

/**
 * MPI write with int data type
 * return 1 if OK, 0 on error
 */
int mpio_write_with_int(MPI_File my_fh, int my_rank, int nb_int, int * my_buf)
{
    MPI_Status status;
    MPI_Offset offset = my_rank * nb_int;
    MPI_File_write_at(my_fh, offset, my_buf,  nb_int, MPI_INT, &status);

    if (status.MPI_ERROR != MPI_SUCCESS) 
        return 0;
    return 1;
}

/**
 *  Main part of the code: call the various read / write function
 * 
 */
int main(int argc, char **argv)
{
    int rank, nb_rank, i;
    MPI_File fh;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_rank);
    int nb_elt = MAX_ELT;

    /*
     * Register the Mpi datatype cell_type in the comm_world
     */
    MPI_Aint offsets[2], extent;
    MPI_Datatype field_types[2]={MPI_INT,MPI_DOUBLE};
    int field_count[2]={2,4};
    /* Need to first figure offset by getting size of MPI_INT */
    MPI_Type_extent(MPI_INT, &extent);
    offsets[0] = 0;
    offsets[1] = 2 * extent;
    /* Now define structured type and commit it */
    MPI_Type_struct(2, field_count, offsets, field_types, &CellType);
    MPI_Type_commit(&CellType);

    int struct_test = 0; 
    int char_test = 0; 
    int int_test = 0;
    /* parsing command line option */
    char opt;
    int verbose = 0;
    while ((opt = getopt(argc, argv, "vn:cipsa")) != -1) 
    {
        switch (opt) 
        {
            case 'v': verbose = 1;
                   break;
            case 'n': nb_elt = atoi(optarg);
                   break;
            case 'c': char_test = 1;
                   break;
            case 'i': int_test = 1;
                   break;
            case 's': struct_test = 1;
                   break;
            case 'a': struct_test = 1; char_test = 1; int_test = 1;
                   break;
            case '?': if(!rank)
                     {
                        fprintf(stderr, "Usage: %s [-v] verbose mode\n   [-n NB_ELT] nb elements per rank\n",  argv[0]);
                        fprintf(stderr, "   [-c] test char MPI I/O mode\n   [-i] test int MPI I/O mode\n");
                        fprintf(stderr, "   [-s] test structure data type MPI I/O mode\n");
                        fprintf(stderr, "   [-a] all types of access\n");
                     }
                   exit(EXIT_FAILURE);
               
         }
    }
    if (nb_elt > MAX_ELT)
        {
        if (!rank) fprintf(stderr, "Please specify a number of elements < %d\n", nb_elt, MAX_ELT);
        exit(EXIT_FAILURE);
        }
    if(!rank) fprintf(stderr, "Running with %d ranks, %d elements per rank\n", nb_rank, nb_elt);

    /* by default enable all tests */
    if (!struct_test && !char_test && !int_test)
    {
        struct_test = 1; char_test = 1; int_test = 1; 
    }
    if (!rank) 
    {
        if (char_test)   fprintf(stderr, "Enabling test for MPI I/O char\n");
        if (int_test)    fprintf(stderr, "Enabling test for MPI I/O integer\n");
        if (struct_test) fprintf(stderr, "Enabling test for MPI I/O Struct complex data type\n");
        if (verbose)     fprintf(stderr, "VERBOSE mode switched on\n");
    }
    /*
     * Proceed to memory allocation
     */
    int *read_int_buffer = (int *) malloc(sizeof(int) * nb_elt);
    char *read_char_buffer = (char *) malloc(sizeof(char) * nb_elt);
    char *write_char_buffer = (char *) malloc(sizeof(char) * nb_elt);
    int *write_int_buffer = (int *) malloc(sizeof(int) * nb_elt);
    Cell_t *my_read_cells = (Cell_t *) malloc(sizeof(Cell_t) * nb_elt);

    /*
     * Reset the status of all possible test file
     */
    if(!rank) fprintf(stderr, "reseting status of input/output files\n");
    char pchar_fname[128]; 
    char pint_fname[128]; 
    char mpchar_fname[128]; 
    char mpint_fname[128]; 
    char mpstruct_fname[128]; 

    sprintf(pchar_fname, "data/fwrite_char_rank_%d", rank);
    sprintf(pint_fname, "data/fwrite_int_rank_%d", rank);
    sprintf(mpchar_fname, "data/mpio_char_shared");
    sprintf(mpint_fname, "data/mpio_int_shared");
    sprintf(mpstruct_fname, "data/mpio_struct_shared");
    if(!rank)
    {
    system("truncate data/mpio_char_shared --size 0");
    system("truncate data/mpio_int_shared --size 0");
    system("truncate data/mpio_struct_shared --size 0");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /*
     * Proceed to MPI I/O tests on a SHARED file
     */
    /* Write and then Read char files */
    if (char_test)
    {
        memset(read_char_buffer, 0, sizeof(char) * nb_elt);
	    for (i =0 ; i < nb_elt ; i++)
            write_char_buffer[i] =  'A' + (i % (26 + rank));
        if(!rank && verbose) dump_char_buffer(write_char_buffer, nb_elt);

        printf("rank: %d, opening %s char file(MPIO Write) \n", rank, mpchar_fname);
        MPI_File_open(MPI_COMM_WORLD, mpchar_fname, MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        mpio_write_with_char(fh, rank, nb_elt, write_char_buffer);
        MPI_File_close(&fh);

        // Read using seek 
        printf("rank: %d, opening shared Char file %s\n", rank, mpchar_fname);
        MPI_File_open(MPI_COMM_WORLD, mpchar_fname, MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
        memset(read_char_buffer, 0, sizeof(char) * nb_elt);
        mpio_read_char_with_seek(fh, rank, nb_elt, read_char_buffer);
        MPI_File_close(&fh);

        printf ("rank: %d seek and read %d char:\n",rank, nb_elt);
        if(!rank && verbose) dump_char_buffer(read_char_buffer, nb_elt);

        memset(read_char_buffer, 0, sizeof(void) * nb_elt);
        printf("rank: %d, opening Char file %s\n", rank, mpchar_fname);
        // Read using at offset
        MPI_File_open(MPI_COMM_WORLD, mpchar_fname, MPI_MODE_RDONLY,MPI_INFO_NULL,&fh);
        mpio_read_char_with_read_at(fh, rank, nb_elt, read_char_buffer);
        MPI_File_close(&fh);
        printf ("rank: %d read at %d char:\n",rank, nb_elt);
        if(!rank && verbose) dump_char_buffer(read_char_buffer, nb_elt);
    }

    /* Read and Write int on a SHARED file files */
    if (int_test)
    {
        memset(write_int_buffer, 0, sizeof(int) * nb_elt);
	    for (i =0 ; i < nb_elt ; i++)
            write_int_buffer[i] =  0 + (i % (26 + rank));
        if(!rank && verbose) dump_int_buffer(write_int_buffer, nb_elt);

        printf("rank: %d, opening %s int file (MPIO Write)\n", rank, mpint_fname);
        MPI_File_open(MPI_COMM_WORLD, mpint_fname, MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        mpio_write_with_int(fh, rank, nb_elt, write_int_buffer);
        MPI_File_close(&fh);

        // Read the new written file
        memset(read_int_buffer, 0, sizeof(int) * nb_elt);
        printf("rank: %d, opening %s int file (MPIO Read)\n", rank, mpint_fname);
        MPI_File_open(MPI_COMM_WORLD, mpint_fname, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
        mpio_read_int_with_seek(fh, rank, nb_elt, read_int_buffer);
        MPI_File_close(&fh);
        printf ("rank: %d read at %d int:\n",rank, nb_elt);
        if (!rank && verbose) dump_int_buffer(read_int_buffer, nb_elt);

    }

    /* Proceed to complex data type writing / reading */
    if (struct_test)
    {
        populate_domain(my_domain, rank, nb_elt) ;
        if (!rank && verbose) dump_domain(my_domain, nb_elt);

        printf("rank: %d, opening %s user define MPI Struct type SHARED (MPIO Write)\n", rank, mpstruct_fname);
        MPI_File_open(MPI_COMM_WORLD, mpstruct_fname, MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        mpio_write_with_datatype(fh, rank, nb_elt, my_domain);
        MPI_File_close(&fh);

        /* Reading the newly written binary file */
        memset((void*)my_read_cells, 0, (size_t) sizeof(Cell_t) * nb_elt);
        printf("rank: %d, opening %s  user define MPI Struct type SHARED file (MPIO Read)\n", rank, mpstruct_fname);
        MPI_File_open(MPI_COMM_WORLD, mpstruct_fname, MPI_MODE_RDWR, MPI_INFO_NULL, &fh);
        mpio_read_with_datatype(fh, rank, nb_elt, my_read_cells);
        MPI_File_close(&fh);
        printf ("rank: %d read at %d MPI struct:\n",rank, nb_elt);
        if (!rank && verbose) dump_domain(my_read_cells, nb_elt);
    }


    MPI_Finalize();
    return 0;
}
