CC=mpicc

OPTFLAGS = -O2
CFLAGS = $(OPTFLAGS) -g -Wall

all: simpleio simpleio_ssf simpleio_mpiio simpleio_md

simpleio: src/simpleio.c
	${CC} -o src/simpleio src/simpleio.c

simpleio_ssf: src/simpleio_ssf.c
	${CC} -o src/simpleio_ssf src/simpleio_ssf.c

simpleio_md: src/simpleio_md.c
	${CC} -o src/simpleio_md src/simpleio_md.c

simpleio_mpiio: src/simpleio_mpiio.c
	${CC} -o src/simpleio_mpiio src/simpleio_mpiio.c

clean:
	rm src/simpleio src/simpleio_ssf src/simpleio_mpiio
