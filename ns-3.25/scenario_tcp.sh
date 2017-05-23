#!/bin/bash

mkdir Tcp1MBResults
cd Tcp1MBResults
mkdir log_Tcp_10_20_3 log_Tcp_10_30_3 log_Tcp_10_40_3
mkdir trace_Tcp_10_20_3 trace_Tcp_10_30_3 trace_Tcp_10_40_3
cd ..

for i in $(seq 1 $1):
do
        
        ./waf --run "scratch/tcp_random_topology --maxBytes=1024000 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=Tcp_trace_10_20_3_$i.tr --nSeed=$i" > log_Tcp_10_20_3_$i.txt 2>&1;
        mv log_Tcp_10_20_3_$i.txt Tcp1MBResults/log_Tcp_10_20_3/;
        mv Tcp_trace_10_20_3_$i.tr Tcp1MBResults/trace_Tcp_10_20_3/;

         ./waf --run "scratch/tcp_random_topology --maxBytes=1024000 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=Tcp_trace_10_30_3_$i.tr --nSeed=$i" > log_Tcp_10_30_3_$i.txt 2>&1;
        mv log_Tcp_10_30_3_$i.txt Tcp1MBResults/log_Tcp_10_30_3/;
        mv Tcp_trace_10_30_3_$i.tr Tcp1MBResults/trace_Tcp_10_30_3/;

         ./waf --run "scratch/tcp_random_topology --maxBytes=1024000 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=Tcp_trace_10_40_3_$i.tr --nSeed=$i" > log_Tcp_10_40_3_$i.txt 2>&1;
        mv log_Tcp_10_40_3_$i.txt Tcp1MBResults/log_Tcp_10_40_3/;
        mv Tcp_trace_10_40_3_$i.tr Tcp1MBResults/trace_Tcp_10_40_3/;

done


