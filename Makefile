all: poke

poke: poke327.o heap.o character.o io.o
	g++ -lncurses poke327.o heap.o character.o io.o -o poke

poke327.o: poke327.cpp heap.h io.h
	g++ -Wall -Werror -g poke327.cpp -c

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

character.o: character.cpp poke327.h io.h
	g++ -Wall -Werror -g character.cpp -c

io.o: io.cpp io.h poke327.h
	g++ -Wall -Werror -g io.cpp -c

tar:
	cd ..
	tar cvfz hall_noah.assignment-1.07.tar.gz hall_noah.assignment-1.07

run:
	./poke

clean:
	rm -f *.o poke
