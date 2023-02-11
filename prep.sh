#! /usr/bin/bash

# Argument validity check
if ! [[ $1 =~ ^[0-9]+$ ]] || ! [[ $2 =~ ^[0-9]+$ ]]; then
    echo "Usage: $0 [NUM_RECORDS <uint>] [NUM_NODES <uint>] [[OPTIONAL] PATH <string>]"
    exit 1
fi

if [[ $# = 3 ]] && ! [[ -x $3 ]]; then
    echo "Error: $3 is not executable"
    exit 1
fi

exe=$(which gensort)
if [[ $# = 3 ]]; then
    exe=$3
fi
if [[ -z $exe ]]; then
    echo "Error: Could not locate the executable gensort"
    exit 1
elif ! [[ -x $exe ]]; then
    echo "Error: Could not execute $exe"
    exit 1
fi

# Data files generation
echo "Preparing sliced data files for distributed sorting..."

min_per_node=100                            # minimum # records per node
real_per_node=$(expr $1 / $2)               # actual # records per node
if [[ $(expr $1 % $2) -ne 0 ]]; then
    real_per_node=$(expr $1 / $2 + 1)               
fi
if [[ $real_per_node -lt $min_per_node ]]; then                  
    echo "Error: too few data or too many nodes"
    exit 1
fi

begin=0
suffix=.dat
for (( i = 1; i <= $2 && $begin < $1; i++ ))
do
    file=p$i$suffix
    amount=$real_per_node
    end=$(expr $begin + $amount)
    if [[ $end -gt $1 ]]; then
        amount=$(expr $1 - $begin)
    fi

    $exe -b$begin -a $amount $file
    begin=$(expr $begin + $amount)
    echo "Generated $amount records in $file"
done

# Summary
total=$(expr $1 \* 100)
str="$total bytes"
if [[ $(( $total / 1000)) -ge 1 ]]; then
    str="$(( $total / 1000)) kilobytes"
fi
if [[ $(( $total / 1000 / 1000 )) -ge 1 ]]; then
    str="$(( $total / 1000 / 1000 )) megabytes"
fi
if [[ $(( $total / 1000 / 1000 / 1000 )) -ge 1 ]]; then
    str="$(( $total / 1000 / 1000 / 1000 )) gigabytes"
fi

echo "$2 data files have been generated; their total size is about $str."