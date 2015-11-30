#!/bin/bash

#PBS -N mpi-io
#PBS -l walltime=00:01:00
#PBS -l nodes=1:ppn=20
#PBS -q reserved4

cd $PBS_O_WORKDIR

module load openmpi/1.8.3/gnu/4.9.2

mpirun -np 4 last_write.x # write_using_set_view.x
# mpirun -np 4 readFile_pointer.x #writeFile_pointer.x #readFile_offset.x #writeFile_offset.x