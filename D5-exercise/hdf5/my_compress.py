import numpy as np
import h5py
import time
import os

print("\n\tGZIP COMPRESSION\n")
myfile = h5py.File("compressed_gzip.h5", "w")

dset = myfile.create_dataset("gzip_dataset", (10000, 10000, 1), maxshape=(100000,100000,None,), chunks = (100, 100, 1), compression = 'gzip')

rnd = np.random.rand(dset.shape[0], dset.shape[1], dset.shape[2])
start = time.clock()

cwd = os.getcwd()

fname = cwd + '/' + myfile.filename

dset[...]  = rnd

myfile.close()
end = time.clock()
time1 = end - start

size1 = os.path.getsize(fname) / 10**6 # size file in MB
print("\tfile: ", fname[len(cwd)+1:], ";  size: ", size1, "MB")
print("\ttotal time: %g s" % time1)

print("\n\tLZF COMPRESSION\n")
my2file = h5py.File("compressed_lzf.h5", "w")

fname = cwd + '/' + my2file.filename
dset2 = my2file.create_dataset("lzf_dataset", (10000, 10000, 1), maxshape=(100000,100000,None,), chunks = (100, 100, 1), compression = 'lzf')

start = time.clock()
dset2[...] = rnd
my2file.close()
end = time.clock()

time2 = end - start
size2 = os.path.getsize(fname) / 10**6 # size file in MB

print("\tfile: ", fname[len(cwd)+1:], ";  size: ", size2, "MB")
print("\ttotal time needed: %g s" % time2)

print("\n\tNO COMPRESSION\n")
mylastfile = h5py.File("uncompressed.h5", "w")

fname = cwd + '/' + mylastfile.filename
dset3 = mylastfile.create_dataset("dataset", (10000, 10000, 1), maxshape=(100000,100000,None,), chunks = (100, 100, 1))

start = time.clock()
dset3[...] = rnd
mylastfile.close()
end = time.clock()

time3 = end - start
size3 = os.path.getsize(fname) / 10**6 # size file in MB

print("\tfile: ", fname[len(cwd)+1:], ";  size: ", size3, "MB")
print("\n\ttotal time: %g s" % time3)


