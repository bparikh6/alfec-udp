#!/bin/bash

mkdir Alfec1MBResults
cd Alfec1MBResults
mkdir log_alfec_10_20_3 log_alfec_10_30_3 log_alfec_10_40_3
mkdir trace_alfec_10_20_3 trace_alfec_10_30_3 trace_alfec_10_40_3
cd ..

for i in $(seq 1 $1):
do
        
        ./waf --run "scratch/alfec_random_topology --nTransLen=5120000000 --nBlocks=5000 --numNodes=50 --numLinks=100 --numSrc=3 --traceName=alfec_trace_10_20_3_$i.tr --nSeed=$i" > log_alfec_10_20_3_$i.txt 2>&1;
        mv log_alfec_10_20_3_$i.txt Alfec1MBResults/log_alfec_10_20_3/;
        mv alfec_trace_10_20_3_$i.tr Alfec1MBResults/trace_alfec_10_20_3/;

         ./waf --run "scratch/alfec_random_topology --nTransLen=1024000 --nBlocks=1 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=alfec_trace_10_30_3_$i.tr --nSeed=$i" > log_alfec_10_30_3_$i.txt 2>&1;
        mv log_alfec_10_30_3_$i.txt Alfec1MBResults/log_alfec_10_30_3/;
        mv alfec_trace_10_30_3_$i.tr Alfec1MBResults/trace_alfec_10_30_3/;

         ./waf --run "scratch/alfec_random_topology --nTransLen=1024000 --nBlocks=1 --numNodes=10 --numLinks=20 --numSrc=3 --traceName=alfec_trace_10_40_3_$i.tr --nSeed=$i" > log_alfec_10_40_3_$i.txt 2>&1;
        mv log_alfec_10_40_3_$i.txt Alfec1MBResults/log_alfec_10_40_3/;
        mv alfec_trace_10_40_3_$i.tr Alfec1MBResults/trace_alfec_10_40_3/;

done


