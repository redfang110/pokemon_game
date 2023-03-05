all: poke

poke: poke327.o heap.o character.o io.o
	g++ -lncurses poke327.o heap.o character.o io.o -o poke

poke327.o: poke327.cpp heap.h character.hpp io.hpp
	g++ -Wall -Werror -g poke327.cpp -c

heap.o: heap.c heap.h
	gcc -Wall -Werror -g heap.c -c

character.o: character.cpp poke327.hpp io.hpp
	g++ -Wall -Werror -g character.cpp -c

io.o: io.cpp io.hpp character.hpp poke327.hpp
	g++ -Wall -Werror -g io.cpp -c

tar:
	cd ..
	tar cvfz hall_noah.assignment-1.06.tar.gz hall_noah.assignment-1.06

run:
	./poke

clean:
	rm -f *.o poke
