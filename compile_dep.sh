#!/bin/bash

mkdir dep
cd dep

git clone https://github.com/marekpiotrow/UWrMaxSat uwrmaxsat

git clone https://github.com/marekpiotrow/cominisatps
cd cominisatps
rm core simp mtl utils && ln -s minisat/core minisat/simp minisat/mtl minisat/utils .
make lr
cp build/release/lib/libcominisatps.a ../../bin/

cd ..
git clone https://github.com/Laakeri/maxpre
cd maxpre
sed -i 's/-g/-D NDEBUG/' src/Makefile
make lib
cp src/lib/libmaxpre.a ../../bin/

cd ../uwrmaxsat
make config
USESCIP= make r
cp build/release/lib/libuwrmaxsat.a ../../bin/