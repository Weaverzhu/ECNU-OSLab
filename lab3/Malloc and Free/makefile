libmem.so : mem.o
	gcc -shared -o libmem.so mem.o
mem.o : mem.c
	gcc -c -fpic mem.c -Wall -Werror
clean : 
	rm -f *.o
	rm -f *.so
	rm -f main
	rm -f ./bin/*
test : mymain.c
	mkdir -p bin
	gcc -L. -o ./bin/test.out mymain.c -Wall -lmem
random: random.c
	mkdir -p bin
	gcc -L. -o ./bin/test.out random.c -Wall -lmem
