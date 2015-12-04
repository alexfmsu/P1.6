#!/bin/bash

echo -e "\nBuilding WholeFileRecordReader.java..."
javac -classpath /usr/lib/hadoop-0.20/hadoop-core-0.20.2-cdh3u1.jar -d RootOnHadoop_classes WholeFileRecordReader.java

echo -e "\nBuilding WholeFileInputFormat.java..."
javac -classpath /usr/lib/hadoop-0.20/hadoop-core-0.20.2-cdh3u1.jar:RootOnHadoop_classes -d RootOnHadoop_classes WholeFileInputFormat.java

echo -e "\nBuilding RootOnHadoop.java..."
javac -classpath /usr/lib/hadoop-0.20/hadoop-core-0.20.2-cdh3u1.jar:RootOnHadoop_classes -d RootOnHadoop_classes RootOnHadoop.java

echo -e "\nPackaging..."
jar -cvf ./RootOnHadoop.jar -C RootOnHadoop_classes/ .
