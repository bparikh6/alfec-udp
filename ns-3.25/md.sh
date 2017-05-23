#!/bin/bash

for i in $(seq 1 $1):
do
        
        ./waf --run "scratch/tcp_random_topology --maxBytes=1024000 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=Tcp_trace_10_20_3_$i.tr --nSeed=$i" > log_Tcp_10_20_3_$i.txt 2>&1;
        mv log_Tcp_10_20_3_$i.txt Tcp1MBResults/log_Tcp_10_20_3/;
        mv Tcp_trace_10_20_3_$i.tr Tcp1MBResults/trace_Tcp_10_20_3/;
        
done
