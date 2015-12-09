#!/bin/bash
DATE=$(date +"%Y%m%d-%H%M%S")
echo "Placing output in /out/$DATE"
hadoop jar /usr/share/hadoop/contrib/streaming/hadoop-*streaming*.jar -file mapper.py -mapper mapper.py -file reducer.py -reducer reducer.py -input /ex1_data/data -output /out/$DATE

