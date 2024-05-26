#!/bin/bash

for file in $(find instances/exact/ -type f | sort -V)
do
    echo $(basename $file)
    echo "" > g.sol
    timeout -k 1 60s ./exact $file g.sol 2> /dev/null
    n=$(cat g.sol | wc -l)
    if (($n > 1))
    then
        pace2024verifier -c $file g.sol
    else
        echo tle
    fi
    #pace2024verifier -c $file solutions/exact/$(basename $file .gr).sol
done

rm g.sol