all: poke

poke: poke327.o heap.o
	gcc poke327.o heap.o -o poke

poke327.o: poke327.c heap.h
	gcc -Wall -Werror -g poke327.c -c

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

tar:
	cd ..
	tar cvfz hall_noah.assignment-1.03.tar.gz hall_noah.assignment-1.03

run:
	./poke

clean:
	rm -f *.o poke