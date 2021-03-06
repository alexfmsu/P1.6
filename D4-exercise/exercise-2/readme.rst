==============
ROOT on Hadoop      
==============


.. contents::


Important note: this is a quick summary which provides the concepts and code examples to allow running ROOT on Hadoop via a MapReduce Job. The example code is meant only 
for testing purpose (exceptions are not handled, some parameters are hardcoded, etc..), so please take it as is :)

For more "hi-level" details please refer to the following paper: S.A. Russo, Running a typical ROOT HEP analysis on Hadoop MapReduce, 2014 J. Phys.: Conf. Ser. 513 032080

Quickstart
==========

The code for running a binary program which access binary data, for “historical” reason is named “RootOnHadoop”

- Create a directory on HDFS, then 3 files inside it

::

	hadoop fs -mkdir /test
	hadoop fs -cp somefile /test/somefile1
	hadoop fs -cp somefile /test/somefile2
	hadoop fs -cp somefile /test/somefile3

- Run the code with the demo command provided here_ 
- Check output



Prerequisites
=============


1. Hadoop command line client working on every node, and in particular "hadoop fs" and "hadoop fsck". Linux/Unix platform.

2. One of the the following two:

   1. FUSE access method: HDFS mounted via FUSE on every node, or

   2. Direct access method: HDFS data directories (dfs.data.dir) accessible by the user running the MapReduce task. In my tests it's 15-20% faster than FUSE.

.. Note:: to implement this methood, the "find" command is used to locate the blocks' files (which have unique file names) within the HDFS data directories on the nodes. It works fine but there must be way to obtain the filename of a block using the HDFS api, even if I still did not find it. You can imporove it by using the "locate" command instead of the find, and by using "find" as the second choiche (the "locate" index might not be updated").

.. _3:

3. ROOT binary files of the dataset to analyze stored in only one block (avoiding to chunk them). This is to take advantage of data locality. You can split your ROOT dataset in files smaller than your HDFS block size or you can set a custom block size for every file, to be set bigger than the file. Not mandatory for testing purpose without data locality. See the bottom of this page for a script containing hints about how to do this step automatically. (n.b. problems for blocks/files greater than 2 Gb, check this out: BUG: Anyone use block size more than 2GB before?). 

The method
==========

The idea is to run within the Map and Reduce task a custom script (like Hadoop Streaming), and to bypass Hadoop's data access methods, allowing to access Map's input data from a standard file system. Every Map task will process an entire ROOT file and not only one chunk, as the standard MapReduce approach wants. If you stored your ROOT files according to the previous point 3_ then processing one file or one chunk is the same thing.


The testing code
================ 

1. You can find RootOnHadoop, in your repo, under :: 

	D4-exercise5/provided_code/RootOnHadoop

2. The code has been compiled for using the method 2.2 (direct access). If it is fine for you, you can use it as-is and go to point 3. If you want to switch to the FUSE access method:

   a. In RootOnHadoop.java search for "int method" and choose your access method by setting the value of this variable. mode=1 means FUSE access (point 2.1), mode=2 is for direct access (point 2.2). If you chose FUSE, also set the var "String fuse_mountpoint" to match the path where you mounted HDFS on the nodes.

   b. Use the build.sh script to build the jar, but edit it to link against the correct Hadoop libraries (the version number is in the library name).

3. Upload a test file in a $input_path_on_HDFS. A text file with a single line is fine for this test.

4. Run the test. Assuming that you are in the root directory of the testing code:

.. _here:

5. The script will performjust an access test (a "ls" command), and you should see an output similar to the one reported below in this page (Map output example and Reduce output example). Check also MapRed/Map.sh and MapRed/Reduce.sh to see what they are doing.::

	hadoop jar RootOnHadoop.jar cern.root.hadoop.RootOnHadoop -map ./MapRed/Map.sh -reduce ./MapRed/Reduce.sh -in $input_path_on_HDFS -out $output_path_on_HDFS

Evolve the Map.sh and Reduce.sh scripts according to the guidelines below, and if you modify the source Java code, remember to run the build script! 

(To tell Hadoop to run on multiple files in multiple directories you can use a syntax like the following in $input_path_on_HDFS: "/data/TOP/\*/\*/\*Egamma\*/\*.root", where whildcards match everything)

Map
===

The script specified with the -map option receives as the first argoument the path of the file to process. This path is a standard file system path (either if you chose to use FUSE or direct acces, and even if a local copy of the file to process was not available).

You can then download in the Map's sandbox your ROOT code and execute is as normal. Just use something like "hadoop fs -get /path/of/ROOT/code ./" in the Map script. Or you can install it on the entire cluster. Or you can provide a support file system like NFS where to store the ROOT code. These topics are covered in detail by the paper, here I just would like to give you the idea.

The output which you want to be transferred to the Reduce task has to have a .out extension. I.e. the file "mydata.out" in the Map's sandbox will be uploaded to the outpur directory as "mapID.mydata.out", to be later accessible by the Reduce task. The stdout and stderr of your Map script is available under the file "mapID.map.out" in the output directory as well.

Reduce
======

The script specified with the -reduce option receives as the first argoument the path of the output (the same path specified with the -out option).

The script also receives on the standard input, one per line, the mapID of the map tasks as they end. You can then access to the Map's output data from the Reduce script by using the path: "$1/mapID.mydata.out" (i.e. "hadoop -fs get $1/mapID.mydata.out").

The stdout and stderr output of the Reduce is saved in the "reduce.out" file in the output directory, if you need to save any other output you have to manually copy it into the output dir (i.e. "hadoop -fs put MyFinalData.root $1/"). Remember that both Map and Reduce sandboxes are cleared after the execution ends. 



Caching
=======

Once the test works, there is another thing to take care of: if the size of the ROOT code you are going to access from the Map task is not negligible in comparizion to the Map's input file, since it needs to be transferred by every Map task, you will end up in vanishing benefits from Hadoop's data locality.

To solve this problem, you can set up a support file system like NFS with an high caching policy (so that the first Map task on the node which access the ROOT code trigger its caching, and the next Map tasks will use the cahced copy) or you implement a cache manually in the Map, using something like:

Set the path for the sw on Hadoop::

	SW_path=/user/sarusso/SW

Where to put the Sw on the nodes for::

	SW_tmp_dir=/tmp/sarusso

Software list, packages are meant to be stored with .tar.gz extension.
The first element in the array has to be the ICToP2 package, the second the GRLib, then everything else that you need to get (if this is the case)::

	SW_packages=(  ICToP2_CutFlow-0.16-1-x86_64 GRLlib-20100414-x86_64 )

Sleep randomly between 0 and 1 sec to avoid concurrencies in the folowing check::

	sleep 0.$[ ( $RANDOM % 10 )  + 1 ]s

Check if we already have the package on the node::

	for SW_package in ${SW_packages[@]}
	do
	if [ ! -d $SW_tmp_dir/$SW_package ]; then

		# Lock the SW package so that another task will not try to dowload it,
		# and will instead wait untill the get&extract is done
		mkdir -p $SW_tmp_dir/$SW_package/LOCK

		echo "Getting $SW_package into $SW_tmp_dir"
		
		# Get via Hadoop FS
		hadoop fs -get $SW_path/$SW_package.tar.gz $SW_tmp_dir/
	
		# Extract
		echo "Extracting $SW_package into $SW_tmp_dir"
		tar -C $SW_tmp_dir/ -zxf $SW_tmp_dir/$SW_package.tar.gz

		# Remove the lock
		rmdir $SW_tmp_dir/$SW_package/LOCK  
	else
		echo "$SW_tmp_dir/$SW_package exists"
	
		while true; do 
		if [ -d $SW_tmp_dir/$SW_package/LOCK ]; then
			echo "$SW_tmp_dir/$SW_package is being processed, waiting..."
			sleep 1
	   	else 
			echo "$SW_tmp_dir/$SW_package is ready"
			break
	    	fi 
		done
	fi 
	done

Job output example
==================

Running::


	hadoop jar RootOnHadoop.jar cern.root.hadoop.RootOnHadoop -map ./MapRed/Map.sh 
	-reduce ./MapRed/Reduce.sh -in /roh/onefile/set/ -out /user/sarusso/roh087

you will get something like::

	Creating temp dir hdfs://lxbsq0929.cern.ch/user/sarusso/roh087/../roh087_tmp ...
	execCmd: Executing "hadoop fs -mkdir hdfs://lxbsq0929.cern.ch/user/sarusso/roh087/../roh087_tmp"

	Uploading map task ...
	execCmd: Executing "hadoop fs -put ./MapRed/Map.sh hdfs://lxbsq0929.cern.ch/user/sarusso/roh087/../roh087_tmp/"

	Uploading reduce task ...
	execCmd: Executing "hadoop fs -put ./MapRed/Reduce.sh hdfs://lxbsq0929.cern.ch/user/sarusso/roh087/../roh087_tmp/"

	12/11/04 19:36:16 INFO mapred.FileInputFormat: Total input paths to process : 1
	12/11/04 19:36:16 INFO mapred.JobClient: Running job: job_201203111220_0772
	12/11/04 19:36:18 INFO mapred.JobClient:  map 0% reduce 0%
	12/11/04 19:36:28 INFO mapred.JobClient:  map 100% reduce 0%
	12/11/04 19:36:48 INFO mapred.JobClient:  map 100% reduce 100%
	12/11/04 19:36:51 INFO mapred.JobClient: Job complete: job_201203111220_0772
	12/11/04 19:36:51 INFO mapred.JobClient: Counters: 22
	12/11/04 19:36:51 INFO mapred.JobClient:   Job Counters 
	12/11/04 19:36:51 INFO mapred.JobClient:     Launched reduce tasks=2
	12/11/04 19:36:51 INFO mapred.JobClient:     SLOTS_MILLIS_MAPS=6397
	12/11/04 19:36:51 INFO mapred.JobClient:     Total time spent by all reduces waiting after reserving slots (ms)=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Total time spent by all maps waiting after reserving slots (ms)=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Launched map tasks=1
	12/11/04 19:36:51 INFO mapred.JobClient:     Data-local map tasks=1
	12/11/04 19:36:51 INFO mapred.JobClient:     SLOTS_MILLIS_REDUCES=15468
	12/11/04 19:36:51 INFO mapred.JobClient:   FileSystemCounters
	12/11/04 19:36:51 INFO mapred.JobClient:     FILE_BYTES_READ=30
	12/11/04 19:36:51 INFO mapred.JobClient:     HDFS_BYTES_READ=138
	12/11/04 19:36:51 INFO mapred.JobClient:     FILE_BYTES_WRITTEN=104960
	12/11/04 19:36:51 INFO mapred.JobClient:   Map-Reduce Framework
	12/11/04 19:36:51 INFO mapred.JobClient:     Reduce input groups=1
	12/11/04 19:36:51 INFO mapred.JobClient:     Combine output records=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Map input records=1
	12/11/04 19:36:51 INFO mapred.JobClient:     Reduce shuffle bytes=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Reduce output records=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Spilled Records=2
	12/11/04 19:36:51 INFO mapred.JobClient:     Map output bytes=6
	12/11/04 19:36:51 INFO mapred.JobClient:     Map input bytes=111312767
	12/11/04 19:36:51 INFO mapred.JobClient:     Combine input records=0
	12/11/04 19:36:51 INFO mapred.JobClient:     Map output records=1
	12/11/04 19:36:51 INFO mapred.JobClient:     SPLIT_RAW_BYTES=138
	12/11/04 19:36:51 INFO mapred.JobClient:     Reduce input records=1

	Removing temp dir ...
	execCmd: Executing "hadoop fs -rmr hdfs://lxbsq0929.cern.ch/user/sarusso/roh087/../roh087_tmp"
	Deleted hdfs://lxbsq0929.cern.ch/user/sarusso/roh087_tmp

Map output example
==================

::

	cat /hdfs/user/sarusso/roh087/0.map.out                                                  

should give something like::

	===== The following is the RootOnHadoop output =====
	Working on /roh/onefile/set/Set1_user.rsoualah.004795._00002.output.root
	Ok, file is made of only one block
	Node IP address: 128.142.172.59
	Replica IP address: 128.142.172.101
	Replica IP address: 128.142.172.59
	Ok, proceed with the local access
	block_id:blk_-8348270279965883918
	find on/data01/hadoop/dfs/data: /data01/hadoop/dfs/data/current/subdir50/subdir34/blk_-8348270279965883918

	find on/data02/hadoop/dfs/data: 
	Input file location=/data01/hadoop/dfs/data/current/subdir50/subdir34/blk_-8348270279965883918

	===== The following is your Map task output =====

	Working on:
	/data01/hadoop/dfs/data/current/subdir50/subdir34/blk_-8348270279965883918

	Access test:
	-rw-r--r-- 1 hdfs hadoop 107M Jan  7  2012 /data01/hadoop/dfs/data/current/subdir50/subdir34/blk_-8348270279965883918


Reduce output example
=====================

::

	cat /hdfs/user/sarusso/roh087/reduce.out

should be something like::

	===== The following is the RootOnHadoop output =====
	===== The following is your Reduce task output =====

	Output dir (containig files to process):
	hdfs://lxbsq0929.cern.ch/user/sarusso/roh087

	Output from Map task #0 is available
	Found 1 items
	-rw-r--r--   2 sarusso supergroup        942 2012-11-04 19:36 /user/sarusso/roh087/0.map.out
	Found 1 items
	-rw-r--r--   2 sarusso supergroup          7 2012-11-04 19:36 /user/sarusso/roh087/0.test.out

	Content of test.out:
	test!!

	Done processing stdin




Sample script to upload files in only one block on HDFS
(n.b. problems for blocks/files greater than 2 Gb, check this out: BUG: Anyone use block size more than 2GB before?).

In the script, the value "512" is the HDFS checksum value (bytes): blockSize should be a multiple of io.bytes.per.checksum so you might have to modify the value according to your cluster settings.

::

	#!/usr/bin/python

	import fileinput
	import math
	import commands
	from subprocess import call
	import os

	for each_line in fileinput.input("MyFileListOneLocalPathPerLine.txt"):

	    print "\n==============================================================="
	    command="ls -l "+each_line    
	    size=float(os.popen(command).read().split(" ")[4])
	    block_size=math.ceil(size/512)*512
	    upload_command="time sudo -u mapred hadoop fs -D dfs.block.size="+str(int(block_size))+" -put "+each_line[:-1]+" /HDFS_upload_path"
	    print upload_command
	    os.system(upload_command)
	    print "===============================================================\n"





