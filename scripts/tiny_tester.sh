#!/bin/bash

for file in $(find instances/tiny/ -type f)
do
    echo $(basename $file .gr)
    ./exact $file tmp.sol
    pace2024verifier -c $file tmp.sol
    pace2024verifier -c $file solutions/tiny/$(basename $file .gr).sol
done

rm tmp.sol