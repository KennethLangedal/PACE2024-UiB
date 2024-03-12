#!/bin/bash

for file in $(find instances/medium/ -type f | sort -V)
do
    echo $(basename $file)
    ./exact $file g.sol
    pace2024verifier -c $file g.sol
    pace2024verifier -c $file solutions/medium/$(basename $file .gr).sol
done

rm tmp.sol