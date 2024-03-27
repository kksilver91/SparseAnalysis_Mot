#!/bin/bash

# RankNUM=$1

ARY_BENCHMARK=('bcsstk32' 'webbase-1M' 'stomach' 'mac_econ_fwd500' 'roadNet-TX' 'com-Youtube' 'wiki-talk-temporal' 'web-Google' 'amazon-2008' 'hollywood-2009')
# ARY_BENCHMARK=('wiki-talk-temporal' 'web-Google' 'amazon-2008' 'hollywood-2009')
ARY_RANKNUM=('2' '4' '8' '16' '32')
ARY_MAP=('4' '5' '6')
# 1: REMAP(noLBBFILL)
# 4: REMAP
# 5: NOMAP 

# ARY_RANKNUM=('32')

set -e

ARY_BENCHMARK_LENGTH=${#ARY_BENCHMARK[@]}
ARY_RANKNUM_LENGHTH=${#ARY_RANKNUM[@]}
ARY_MAP_LENGHTH=${#ARY_MAP[@]}

ary_cnt=0
ary_cnt_RANKNUM=0
ary_cnt_MAP=0

while [ $ary_cnt_MAP -lt $ARY_MAP_LENGHTH ]
do
    while [ $ary_cnt_RANKNUM -lt $ARY_RANKNUM_LENGHTH ]
    do
        while [ $ary_cnt -lt $ARY_BENCHMARK_LENGTH ]
        do
            echo "Generate trace about ${ARY_BENCHMARK[$ary_cnt]}" 


            if [ ! -e benchmark/${ARY_BENCHMARK[$ary_cnt]}.txt ]; then
                echo "There is no ${ARY_BENCHMARK[$ary_cnt]}.txt"
                break
            fi

            ./SPARSE_ANALYSIS_${ARY_MAP[$ary_cnt_MAP]} benchmark/${ARY_BENCHMARK[$ary_cnt]}.txt ${ARY_RANKNUM[$ary_cnt_RANKNUM]}
            wait
            
            ary_cnt=$((ary_cnt+1))
        done
        ary_cnt_RANKNUM=$((ary_cnt_RANKNUM+1))
        ary_cnt=0
    done

    mv *.txt traces_${ARY_MAP[$ary_cnt_MAP]}


    ary_cnt_RANKNUM=0
    ary_cnt_MAP=$((ary_cnt_MAP+1))
done


