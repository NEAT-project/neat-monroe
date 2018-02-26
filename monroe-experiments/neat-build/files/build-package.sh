#!/bin/bash

mkdir -p /mnt/build 
cd /mnt/build
rm -Rf * 
cmake ../ 
make 
make package
