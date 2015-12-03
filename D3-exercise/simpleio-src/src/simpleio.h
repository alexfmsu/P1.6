#ifndef _SIMPLEIO_H
#define _SIMPLEIO_H

/* ioctl to flush cache on a file descriptor, taken from fio */
#ifndef BLKFLSBUF
#define BLKFLSBUF       _IO(0x12,97)
#endif

/* FILESIZE in bytes */
#define FILESIZE        536870912

size_t posix_call_fwrite_int(int my_rank, size_t filesize, char *filename);
size_t posix_call_fread_int(int my_rank, size_t filesize, char *filename);

#endif
