mem: mem.h mem.c
	gcc -c -fpic mem.c mem.h -Wall -Werror
	gcc -shared -o libmem.so mem.o
	gcc mem.c mem.h -Wall -Werror
clean:
	rm -rf mem