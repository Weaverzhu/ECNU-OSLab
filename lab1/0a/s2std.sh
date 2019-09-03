#!/bin/bash
#s2.sh

#list the infomation of files in /bin into tmp
ls -l /bin > tmp

#remove output before
rm -f output

#read each line in tmp
awk '{if($9 >= "b" && $9 < "c") print $9" "$3" "$1}' tmp >> output

#sort the infomation
sort -f -k 1 -t " " output | tee output

#set the permissions of output
chmod 744 output

#remove the tmp file
rm -f tmp