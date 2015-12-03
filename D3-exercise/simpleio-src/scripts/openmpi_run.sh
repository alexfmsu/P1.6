#!/bin/bash

program=${program:-"simpleio"}
hosts=${hosts:-$(hostname -s)}
tpc=${tpc:-1}

module load openmpi-x86_64

echo "Running ${program} on ${hosts} with ${tpc} thread(s) per client"

mpirun --allow-run-as-root --host ${hosts} --map-by ppr:${tpc}:node /lustre/home/simpleio/src/${program}

exit 0
