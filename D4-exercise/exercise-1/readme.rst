D4-exercise4 - Hadoop hands-on
==============================

0. Take a look to the "generate.py" code and understand what it is doing 
1. Run the code "generate.py" to create your data set 
2. Create a folder HDFS 

::

	hadoop fs -mkdir /D4-exercise4

3. Run the following command, to add the to the hadoop filesystem in the freshly created folder

::

	hadoop fs -put {data set filename} /D4-exercise4 

4. Run the script "run.sh". It will execute the command 

:: 

	hadoop jar /usr/share/hadoop/contrib/streaming/hadoop-*streaming*.jar -file mapper.py -mapper mapper.py -file reducer.py -reducer reducer.py -input /D4-exercise4/{data set filename} -output /out/$DATE

"Hadoop jar" runs a job
"/usr/share/hadoop/contrib/streaming/hadoop-*streaming*.jar" is the job itself, and it is the "Hadoop Streaming" component. 
The parameters -file -mapper, -reducer, -input and  -output are flags for Hadoop Streaming, not of Hadoop itself

5. ON YOUR MACHINE, modify /etc/hosts to allow the webinterfaces to work. The updated list of name and ip is available here_

6. Repeat the task and keep an eye on the JobTracker web interface

.. _here : http://elearn.mhpc.it/moodle/mod/lesson/edit.php?id=182&mode=single&pageid=79 
