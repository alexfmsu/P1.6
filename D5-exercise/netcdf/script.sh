#!/bin/bash

# data=data_down.nc.old #file in which are stored the dowloaded data
data=data_down.nc #file in which are stored the dowloaded data
namef=test
ext=.nc
sec_ext=s.nc
tmp_file=tmp.dat
trash=trash.txt

for fname in size.dat s_size.dat timing.dat s_timing.dat
do
    if [ -f $fname ]; then
	echo $fname found. Removing...
	rm $fname
    fi
done

for L in 1 2 3 4 5 6 7 8 9
do

    name=$namef$L$ext
    sec_name=$namef$L$sec_ext

    echo Working on $name and $sec_name

    nccopy -d$L $data $name
    nccopy -d$L -s $data $sec_name
    du -b $name | awk '{print $1}' >> size.dat
    du -b $sec_name | awk '{print $1}' >> s_size.dat

    count=1
    
    while [ $count -le 10 ]
    do
	/usr/bin/time -p -o $tmp_file ncdump $name > /dev/null #$trash
	cat $tmp_file | grep user | awk '{print $2}' >> timing.dat

	/usr/bin/time -p -o $tmp_file ncdump $sec_name > /dev/null #$trash
	cat $tmp_file | grep user | awk '{print $2}' >> s_timing.dat

	# echo $count
	count=$((count+1))
    done
    rm $name $sec_name
done

rm $tmp_file
