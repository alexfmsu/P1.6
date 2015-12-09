1. Get the code

::

  hadoop fs -get /means.py ./

2. Activate the virtual env required for the code

::

 source /home/ubuntu/venv/bin/activate

3. Execute the simulation on the file

::

  python means.py $1

4. Rename the output to be uploaded to HDFS

::

  for file in \*.nc; do
    if [[ $file == *"_mon_"* ]]
       then
       mv "$file" "`basename $file .nc`.out"
    fi
  done
