#!/bin/bash
set -o errexit
python util.py mymain.c
clear
make
make test
./bin/test.out