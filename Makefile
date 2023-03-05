all: world

world: generate_world.c
	gcc -Wall -Werror -g generate_world.c -o world

tar:
	cd ..
	tar cvfz hall_noah.assignment-1.02.tar.gz hall_noah.assignment-1.02

run:
	./world

clean:
	rm -f world
