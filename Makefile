w4118_sh: main.o shell.o queue.o
	gcc -Wall main.o shell.o queue.o -o w4118_sh
	rm *.o
main.o: main.c
	gcc -Wall -O2 -c main.c -o main.o

shell.o: shell.h shell.c
	gcc -Wall -O2 -c shell.c -o shell.o

queue.o: queue.h queue.c
	gcc -Wall -O2 -c queue.c -o queue.o

clean:
	rm *.o w4118_sh
