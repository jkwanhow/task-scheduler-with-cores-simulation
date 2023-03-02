allocate: allocate.c
	gcc -o allocate allocate.c -lm -I./stucture.h

clean:
	rm -f allocate
	rm -f *.o
