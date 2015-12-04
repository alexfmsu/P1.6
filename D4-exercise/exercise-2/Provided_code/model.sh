#!/bin/bash

TEMPLATE=sfc_CAM-44_MOHC-HadGEM2-ES_rcp85_r1i1p1_ICTP-RegCM4-3_v4_3hr_200601010300-200602010000.nc
START=2008
END=2020

for (( y = $START; y < $END; y ++ ))
do
  for (( m = 1; m < 13; m ++ ))
  do
    python2 data_generator.py $TEMPLATE $y $m
  done
done
