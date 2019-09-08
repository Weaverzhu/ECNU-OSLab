gcc -o generate generate.c -Wall -Werror
echo ==========
./generate -s 0 -n 100 -o /tmp/outfile
echo ==========