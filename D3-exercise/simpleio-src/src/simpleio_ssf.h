#ifndef _SIMPLEIO_SSF_H
#define _SIMPLEIO_SSF_H

/* ioctl to flush cache on a file descriptor, taken from fio */
#ifndef BLKFLSBUF
#define BLKFLSBUF       _IO(0x12,97)
#endif

/* MPI root rank */
#define MASTER          0
/* FILESIZE in bytes */
#define FILESIZE        536870912
/* IO SIZE in bytes */
#define IOSIZE          4

size_t posix_call_fwrite_int(int my_rank, size_t size, char *filename, int world_size);
size_t posix_call_fread_int(int my_rank, size_t size, char *filename, int world_size);

#endif
