#!/bin/bash

program=${program:-"simpleio"}
hosts=${hosts:-$(hostname -s)}
threads=${threads:-1}

module load mpich-x86_64

echo "Running ${program} on ${hosts} with ${threads} thread(s)"

mpirun -hosts ${hosts} -n ${threads} /lustre/home/simpleio/src/${program}

exit 0
