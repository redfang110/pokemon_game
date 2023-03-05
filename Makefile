all: map

map: generate_map.c
	gcc -Wall -Werror -g generate_map.c -o map

clean:
	rm -f map
