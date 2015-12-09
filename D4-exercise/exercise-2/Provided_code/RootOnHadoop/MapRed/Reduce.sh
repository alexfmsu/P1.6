#!/bin/bash

echo ""
echo "Output dir (containig files to process):"
echo $1
echo ""

# Read the stdin
while read mapID
do
echo "Output from Map task #$mapID is available"

# Do studd with the file

hadoop fs -ls $1/$mapID.map.out

hadoop fs -ls $1/$mapID.test.out

echo ""
echo "Content of test.out:"
hadoop fs -cat $1/$mapID.test.out

echo ""
done

echo "Done processing stdin"

# Perform some action once all the ouput files from the Map tasks are available, if necessary.
echo ""
