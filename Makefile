all: poke

poke: poke327.o heap.o
	gcc -lncurses poke327.o heap.o -o poke

poke327.o: poke327.c heap.h
	gcc -Wall -Werror -g poke327.c -c

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

tar:
	cd ..
	tar cvfz hall_noah.assignment-1.05.tar.gz hall_noah.assignment-1.05

run:
	./poke

clean:
	rm -f *.o poke
