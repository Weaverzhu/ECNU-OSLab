#!/bin/bash
set -o errexit
gcc mem.h mem.c -o main -Wall
./main
