#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void print_table(int table[21][80]);

void fill(int map[21][80])
{
    int i, j;
    for(i = 0; i < 21; i++) {
        for(j = 0; j < 80; j++) {
            if(map[i][j] == 0){
                map[i][j] = 4;
            }
        }
    }
}

void civalize(int map[21][80], int x, int y)
{
    int i;
    int pokemart = (rand() % 77) + 1;
    int pokecenter = (rand() % 18) + 1;
    //array holds where the road goes off the map. Organized: {Left, Righ, Top, Bottom}. -1 == ungenerated.
    int roadpoint[2];
    if (x == -1) {
        roadpoint[0] = (rand() % 19) + 1;
    } else {
        roadpoint[0] = x;
    }
    if (y == -1) {
        roadpoint[1] = (rand() % 78) + 1;
    } else {
        roadpoint[1] = y;
    }

    //place x-axis road
    for(i = 0; i < 80; i++) {
        map[roadpoint[0]][i] = 5;
    }
    //place y-axis road
    for(i = 0; i < 21; i++) {
        map[i][roadpoint[1]] = 5;
    }

    //makes sure pokemart and pokecenter are not intersecting roads
    if(pokemart > roadpoint[1] - 4 && pokemart < roadpoint[1] + 4)
    {
        if(pokemart < 5) {
            pokemart = pokemart + 5;
        }else{
            pokemart = pokemart - 5;
        }
    }
    if(pokecenter > roadpoint[0] - 2 && pokecenter < roadpoint[0] + 2)
    {
        if(pokecenter < 5) {
            pokecenter = pokecenter + 5;
        }else{
            pokecenter = pokecenter - 5;
        }
    }

    //place pokemart
    if(roadpoint[0] > 3) {
        map[roadpoint[0] - 1][pokemart] = 6;
        map[roadpoint[0] - 2][pokemart] = 6;
        map[roadpoint[0] - 1][pokemart - 1] = 6;
        map[roadpoint[0] - 2][pokemart - 1] = 6;
    }else {
        map[roadpoint[0] + 1][pokemart] = 6;
        map[roadpoint[0] + 2][pokemart] = 6;
        map[roadpoint[0] + 1][pokemart + 1] = 6;
        map[roadpoint[0] + 2][pokemart + 1] = 6;
    }

    //place pokecenter
    if(roadpoint[1] > 3) {
        map[pokecenter][roadpoint[1] - 1] = 7;
        map[pokecenter][roadpoint[1] - 2] = 7;
        map[pokecenter - 1][roadpoint[1] - 1] = 7;
        map[pokecenter - 1][roadpoint[1] - 2] = 7;
    }else {
        map[pokecenter][roadpoint[1] + 1] = 7;
        map[pokecenter][roadpoint[1] + 2] = 7;
        map[pokecenter + 1][roadpoint[1] + 1] = 7;
        map[pokecenter + 1][roadpoint[1] + 2] = 7;
    }

}

void get_grid(int map[21][80], int array[4], int seed)
{
    int i, j, temp;
    array[0] = (rand() % 78) + 1;
    array[1] = (rand() % 19) + 1;
    array[2] = (rand() % 78) + 1;
    array[3] = (rand() % 19) + 1;
    if(array[0] > array[2]){
        temp = array[0];
        array[0] = array[2];
        array[2] = temp;
    }
    if(array[1] > array[3]) {
        temp = array[1];
        array[1] = array[3];
        array[3] = temp;
    }

    for(i = array[1]; i <= array[3]; i++) {
        for(j = array[0]; j <= array[2]; j++) {
            map[i][j] = seed;
        }
    }
}

void generate(int map[21][80], int x, int y) 
{
    //seed arrays store ordered pairs. Organized: {x1, y1, x2, y2, ...}
    int grid[4];
    int i, j;

    for (i = 0; i < 21; i++) {
        for (j = 0; j < 80; j++) {
            map[i][j] = 0;
        }
    }

    for(i = 0; i < 15; i++) {
        get_grid(map, grid, 1);
        get_grid(map, grid, 2);
        get_grid(map, grid, 1);
        get_grid(map, grid, 2);
        get_grid(map, grid, 3);
        get_grid(map, grid, 4); 
    }

    fill(map);
    civalize(map, x, y);

    print_table(map);
}

void print_table(int table[21][80])
{
	int i, j;

	for (i = 0; i < 21; i++) {
		for (j = 0; j < 80; j++) {
            switch(table[i][j]) {
                case 1:
                    printf(". ");
                    break;
                case 2:
                    printf(": ");
                    break;
                case 3:
                    printf("^ ");
                    break;
                case 4:
                    printf("%c ", '%');
                    break;
                case 5:
                    printf("# ");
                    break;
                case 6:
                    printf("M ");
                    break;
                case 7:
                    printf("C ");
                    break;
                default:
                    printf("! ");
            }
		}
		printf("\n");
	}
	printf("\n");
}

struct map {
    //1: short grass, 2: tall grass, 3: trees, 4: rocks, 5: road, 6: pokemart, 7: pokecenter
    int terrain[21][80];
    
    //1-78
    int n_road;
    int s_road;
    //1-19
    int e_road;
    int w_road;
};

struct world {
    struct map *maps[410][410];

    //in terms of 0 -- 409, not -200 –- 200
    int current_x;
    int current_y;
};

int main(int argc, char *argv[]) {
    struct world game;
    // int m[21][80];
    srand(time(NULL));


    generate(m, -1, -1);
}