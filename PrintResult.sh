#!/bin/bash

ARY_BENCHMARK=('ns3Da' 'hollywood-2009' 'com-Youtube' 'road_usa' 'web-Google' 'wiki-talk-temporal' 'cit-Patents' 'F1' 'delaunay_n13' 'delaunay_n19' 'amazon-2008' 'roadNet-TX' 'web-Google' 'mac_econ_fwd500'  'language' 'stomach' 'webbase-1M' 'bcsstk32')
ARY_BENCHMARK_LENGTH=${#ARY_BENCHMARK[@]}

ary_cnt=0

rm result.txt

while [ $ary_cnt -lt $ARY_BENCHMARK_LENGTH ]
do
    echo "print result about ${ARY_BENCHMARK[$ary_cnt]}" 


    if [ ! -e benchmark/${ARY_BENCHMARK[$ary_cnt]}.txt ]; then
        echo "There is no ${ARY_BENCHMARK[$ary_cnt]}.txt"
        break
    fi

    ./SPARSE_ANALYSIS benchmark/${ARY_BENCHMARK[$ary_cnt]}.txt >> result.txt
    wait

    cat result.txt
    
    ary_cnt=$((ary_cnt+1))
done

mv *.txt Result
