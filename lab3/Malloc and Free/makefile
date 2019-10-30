libmem.so : mem.o
	gcc -shared -o libmem.so mem.o
mem.o : mem.c
	gcc -c -fpic mem.c -Wall -Werror
clean : 
	rm -f *.o
	rm -f *.so
	rm -f main
test : mymain.c
	gcc -lmem -L. -o ./bin/test.out mymain.c -Wall -Werror -g