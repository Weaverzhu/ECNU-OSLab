#!/bin/sh
# echo $#
if [ $# -eq 2 ]; then
    gcc $1 -o $2 -Wall
else
    echo "Usage: bash ./go.sh [sourcefile] [outputfile]"
fi