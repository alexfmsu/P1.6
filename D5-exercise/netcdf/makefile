CC=gcc
LIBS=-lnetcdf
SRCS=mslp.c
OBJS=$(SRCS:.c=.o)
EXE=$(SRCS:.c=.x)

all: $(EXE)

mslp.x: mslp.o
	gcc $(LIBS) -o $@ $^

%.o: %.c
	gcc -c $^

clean:
	rm -rf *.o *.x
