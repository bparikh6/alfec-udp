#!/bin/bash

# ------------------------------------------------------
# Argument check
ARGS=1
E_BADARGS=85

if [ $# -ne "$ARGS" ]
then
echo "Usage: `basename $0` seeds"
exit $E_BADARGS
fi
# ------------------------------------------------------


mkdir AlfecResults
#cd Alfec1MBResults
#mkdir log_alfec_10_20_3 log_alfec_10_30_3 log_alfec_10_40_3
#mkdir trace_alfec_10_20_3 trace_alfec_10_30_3 trace_alfec_10_40_3
#cd ..

DataSize=1024000
DS="1MB"
Nodes=100
Links=200
l_offset=50;
Src=5


for i in $(seq 1 $1)
do
        
        ./waf --run "scratch/alfec_random_topology --nTransLen=1024000 --nBlocks=1 --numNodes=$Nodes --numLinks=$Links --numSrc=$Src --traceName=alfec_trace_${DS}_${Nodes}_${Links}_${Src}_${i}.tr --nSeed=$i" > log_alfec_$DS\_$Nodes\_$Links\_$Src\_$i.txt
        mv log_alfec_$DS\_$Nodes\_$Links\_$Src\_$i.txt AlfecResults/;
        mv alfec_trace_$DS\_$Nodes\_$Links\_$Src\_$i.tr AlfecResults/;
        let "Links=$Links+$l_offset"

done


