#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void print_table(int table[21][80]);

typedef struct map_struct {
    //1: short grass, 2: tall grass, 3: trees, 4: rocks, 5: road, 6: pokemart, 7: pokecenter
    int terrain[21][80];

    //coordinates of this map within the world
    int x;
    int y;
    
    //exits
    //1-78
    int n;
    int s;
    //1-19
    int e;
    int w;

    //probability of buildings spawning
    float d;
} map;

typedef struct world_struct {
    map *maps[401][401];

    //in terms of 0 -- 409, not -200 –- 200
    int current_x;
    int current_y;

    //0 = false, 1 = true
    int quit;
} world;

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

void civalize(map *map, int n, int e, int s, int w)
{
    int i, min, max;
    float chance [2];
    int pokemart = (rand() % 18) + 1;
    int pokecenter = (rand() % 77) + 1;
    //array holds where the road goes off the map n, e, s, w. Organized: {Left, Righ, Top, Bottom}. -1 == ungenerated.
    int roadpoint[4];
    int random[2] = {-1, -1};
    chance[0] = (rand() / (float) RAND_MAX);
    chance[1] = (rand() / (float) RAND_MAX);
    
    if (n == -1) {
        if (s == -1) {
            roadpoint[0] = (rand() % 75) + 2;
            roadpoint[2] = roadpoint[0];
        } else {
            roadpoint[2] = s;
            roadpoint[0] = roadpoint[2];
        }
    } else {
        roadpoint[0] = n;
        if (s == -1) {
            roadpoint[2] = roadpoint[0];
        } else {
            roadpoint[2] = s;
        }
    }
    map->n = roadpoint[0];
    map->s = roadpoint[2];

    if (e == -1) {
        if (w == -1) {
            roadpoint[1] = (rand() % 15) + 2;
            roadpoint[3] = roadpoint[1];
        } else {
            roadpoint[3] = w;
            roadpoint[1] = roadpoint[3];
        }
    } else {
        roadpoint[1] = e;
        if (w == -1) {
            roadpoint[3] = roadpoint[1];
        } else {
            roadpoint[3] = w;
        }
    }
    map->e = roadpoint[1];
    map->w = roadpoint[3];

    if(map->y > 0 && map->y < 400) {
        //place y-axis road
        if(map->n == map->s) {
            for(i = 0; i < 21; i++) {
                map->terrain[i][map->n] = 5;
            }
        } else {
            random[0] = (rand() % 10) + 5;
            for(i = 0; i <= random[0]; i++) {
                map->terrain[i][map->n] = 5;
            }
            for(i = 20; i >= random[0]; i--) {
                map->terrain[i][map->s] = 5;
            }

            if(map->n < map->s) {
                min = map->n;
                max = map->s;
            } else {
                min = map->s;
                max = map->n;
            }

            for(i = min; i <= max; i++) {
                map->terrain[random[0]][i] = 5;
            }
        }
    } else {
        if(map->y == 400) {
            if(map->e == map-> w && map->e != -1) {
                for(i = 20; i > map->e; i--) {
                    map->terrain[i][map->s] = 5;
                }
                map->n = -1;
            } else {
                for(i = 20; i > 1; i--) {
                    map->terrain[i][map->s] = 5;
                }
                map->n = -1;
            }
        } else {
            if(map->e == map-> w && map->e != -1) {
                for(i = 0; i < map->e; i++) {
                    map->terrain[i][map->n] = 5;
                }
                map->s = -1;
            } else {
                for(i = 0; i < 20; i++) {
                    map->terrain[i][map->s] = 5;
                }
                map->s = -1;
            }
        }
    }

    if(map->x > 0 && map->x < 400) {
        printf("if gucci N: %d, E: %d, S: %d, W: %d\n", map->n, map->e, map->s, map->w);
        //place x-axis road
        if(map->e == map->w) {
            for(i = 0; i <= 80; i++) {
                map->terrain[map->e][i] = 5;
            }
        } else {
            random[1] = (rand() % 69) + 5;
            for(i = 0; i <= random[1]; i++) {
                map->terrain[map->e][i] = 5;
            }
            for(i = 79; i >= random[1]; i--) {
                map->terrain[map->w][i] = 5;
            }

            if(map->e < map->w) {
                min = map->e;
                max = map->w;
            } else {
                min = map->w;
                max = map->s;
            }

            for(i = min + 1; i < max; i++) {
                map->terrain[i][random[1]] = 5;
                map->terrain[i][random[1]] = 5;
            }
        }
    } else {
        if(map->x == 400) {
            printf("if == 400 N: %d, E: %d, S: %d, W: %d\n", map->n, map->e, map->s, map->w);
            if(map->n == map->s && map->n != -1) {
                for(i = 0; i < map->n; i++) {
                    map->terrain[map->w][i] = 5;
                }
                map->e = -1;
            } else {
                for(i = 0; i < 78; i++) {
                    map->terrain[map->w][i] = 5;
                }
                map->e = -1;
            }
        } else {
            if(map->n == map->s && map->n != -1) {
                for(i = 78; i > map->n; i--) {
                    map->terrain[map->e][i] = 5;
                }
                map->w = -1;
            } else {
                for(i = 78; i > 1; i--) {
                    map->terrain[map->e][i] = 5;
                }
                map->w = -1;
            }
        }
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

    if(chance[0] <= map->d) {
        //place pokemart
        if(map->n > 3) {
            map->terrain[pokemart][map->n - 1] = 6;
            map->terrain[pokemart][map->n - 2] = 6;
            map->terrain[pokemart - 1][map->n - 1] = 6;
            map->terrain[pokemart - 1][map->n - 2] = 6;
        }else {
            map->terrain[pokemart][map->n + 1] = 6;
            map->terrain[pokemart][map->n + 2] = 6;
            map->terrain[pokemart + 1][map->n + 1] = 6;
            map->terrain[pokemart + 1][map->n + 2] = 6;
        }
    }

    if(chance[1] <= map->d) {
        //place pokecenter
        if(map->e > 3) {
            map->terrain[map->e - 1][pokecenter] = 7;
            map->terrain[map->e - 2][pokecenter] = 7;
            map->terrain[map->e - 1][pokecenter - 1] = 7;
            map->terrain[map->e - 2][pokecenter - 1] = 7;
        }else {
            map->terrain[map->e + 1][pokecenter] = 7;
            map->terrain[map->e + 2][pokecenter] = 7;
            map->terrain[map->e + 1][pokecenter + 1] = 7;
            map->terrain[map->e + 2][pokecenter + 1] = 7;
        }
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

//x is road laying accross the x-axis (1-19), y is road laying accross the y-axis (1-78)
void generate(map *map, int n, int e, int s, int w) 
{
    //seed arrays store ordered pairs. Organized: {x1, y1, x2, y2, ...}
    int grid[4];
    int i, j;

    if(n != -1) {
        map->n = n;
    }
    if(e != -1) {
        map->e = e;
    }
    if(s != -1) {
        map->s = s;
    }
    if(w != -1) {
        map->w = w;
    }

    for (i = 0; i < 21; i++) {
        for (j = 0; j < 80; j++) {
            map->terrain[i][j] = 0;
        }
    }

    for(i = 0; i < 15; i++) {
        get_grid(map->terrain, grid, 1);
        get_grid(map->terrain, grid, 2);
        get_grid(map->terrain, grid, 1);
        get_grid(map->terrain, grid, 2);
    }
    get_grid(map->terrain, grid, 3);
    get_grid(map->terrain, grid, 4); 
    get_grid(map->terrain, grid, 3);
    get_grid(map->terrain, grid, 4); 

    fill(map->terrain);

    civalize(map, n, e, s, w);
}

void print_table(int table[21][80])
{
	int i, j;
    printf("\n");

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
}

void check_direction(world *game, int ar[4], int x, int y) 
{
    //north
    if((game->maps[y + 1][x]) == NULL) {
        ar[0] = -1;
    } else {
        ar[0] = game->maps[y + 1][x]->s;
    }
    
    //east
    if((game->maps[y][x + 1]) == NULL) {
        ar[1] = -1;
    } else {
        ar[1] = game->maps[y][x + 1]->w;
    }

    //south
    if((game->maps[y - 1][x]) == NULL) {
        ar[2] = -1;
    } else {
        ar[2] = game->maps[y - 1][x]->n;
    }

    //west
    if((game->maps[y][x - 1]) == NULL) {
        ar[3] = -1;
    } else {
        ar[3] = game->maps[y][x - 1]->e;
    }
}

void menu(world *game, char c) 
{
    int desired_x = game->current_x;
    int desired_y = game->current_y;
    int directions[4];
    switch(c) {
        case 'n':
            desired_y++;
            if(desired_y > 400) {
                printf("You have reached the edge of the map\n");
                return;
            }
            if(!(game->maps[desired_y][desired_x])) {
                game->maps[desired_y][desired_x] = malloc(sizeof(map));
                game->maps[desired_y][desired_x]->d = ((-45.00 * (abs(200 - desired_x) + abs(200 - desired_y)) / 200.00) + 50.00) / 100.00;
                game->maps[desired_y][desired_x]->x = desired_x;
                game->maps[desired_y][desired_x]->y = desired_y;
                check_direction(game, directions, desired_x, desired_y);
                generate(game->maps[desired_y][desired_x], directions[0], directions[1], directions[2], directions[3]);
            }
            game->current_x = desired_x;
            game->current_y = desired_y;
            break;
        case 's':
            desired_y--;
            if(desired_y < 0) {
                printf("You have reached the edge of the map\n");
                return;
            }
            if(!(game->maps[desired_y][desired_x])) {
                game->maps[desired_y][desired_x] = malloc(sizeof(map));
                game->maps[desired_y][desired_x]->d = ((-45.00 * (abs(200 - desired_x) + abs(200 - desired_y)) / 200.00) + 50.00) / 100.00;
                game->maps[desired_y][desired_x]->x = desired_x;
                game->maps[desired_y][desired_x]->y = desired_y;
                check_direction(game, directions, desired_x, desired_y);
                generate(game->maps[desired_y][desired_x], directions[0], directions[1], directions[2], directions[3]);
            }
            game->current_x = desired_x;
            game->current_y = desired_y;
            break;
        case 'e':
            desired_x++;
            if(desired_x > 400) {
                printf("You have reached the edge of the map\n");
                return;
            }
            if(!(game->maps[desired_y][desired_x])) {
                game->maps[desired_y][desired_x] = malloc(sizeof(map));
                game->maps[desired_y][desired_x]->d = ((-45.00 * (abs(200 - desired_x) + abs(200 - desired_y)) / 200.00) + 50.00) / 100.00;
                game->maps[desired_y][desired_x]->x = desired_x;
                game->maps[desired_y][desired_x]->y = desired_y;
                check_direction(game, directions, desired_x, desired_y);
                generate(game->maps[desired_y][desired_x], directions[0], directions[1], directions[2], directions[3]);
            }
            game->current_x = desired_x;
            game->current_y = desired_y;
            break;
        case 'w':
            desired_x--;
            if(desired_x < 0) {
                printf("You have reached the edge of the map\n");
                return;
            }
            if(!(game->maps[desired_y][desired_x])) {
                game->maps[desired_y][desired_x] = malloc(sizeof(map));
                game->maps[desired_y][desired_x]->d = ((-45.00 * (abs(200 - desired_x) + abs(200 - desired_y)) / 200.00) + 50.00) / 100.00;
                game->maps[desired_y][desired_x]->x = desired_x;
                game->maps[desired_y][desired_x]->y = desired_y;
                check_direction(game, directions, desired_x, desired_y);
                generate(game->maps[desired_y][desired_x], directions[0], directions[1], directions[2], directions[3]);
            }
            game->current_x = desired_x;
            game->current_y = desired_y;
            break;
        case 'f':
            scanf(" %d %d", &desired_x, &desired_y);
            printf("Flying to (%d, %d)\n", desired_x, desired_y);
            desired_x = 200 + desired_x;
            desired_y = 200 + desired_y;
            if(desired_x < 0 || desired_x > 401 || desired_y < 0 || desired_y > 401) {
                printf("You are trying to fly out of bounds...\n");
                return;
            }
            if(!(game->maps[desired_y][desired_x])) {
                game->maps[desired_y][desired_x] = malloc(sizeof(map));
                game->maps[desired_y][desired_x]->d = ((-45.00 * (abs(200 - desired_x) + abs(200 - desired_y)) / 200.00) + 50.00) / 100.00;
                game->maps[desired_y][desired_x]->x = desired_x;
                game->maps[desired_y][desired_x]->y = desired_y;
                check_direction(game, directions, desired_x, desired_y);
                generate(game->maps[desired_y][desired_x], directions[0], directions[1], directions[2], directions[3]);
            }
            game->current_x = desired_x;
            game->current_y = desired_y;
            break;
        case 'q':
            game->quit = 1;
            break;
        default:
            printf("%c is not a valid input\n", c);
            break;
    }
}

void teardown(world *game)
{
    int i, j;

    for (i = 0; i < 401; i++) {
        for (j = 0; j < 401; j++) {
            free(game->maps[i][j]);
        }
    }
    free(game);
}

int main(int argc, char *argv[]) {
    world *game;
    char c;
    srand(time(NULL));

    game = malloc(sizeof(world));

    //initialize to start
    game->current_x = 200;
    game->current_y = 200;
    game->quit = 0;
    game->maps[200][200] = malloc(sizeof(map));
    game->maps[200][200]->d = (50.00) / 100.00;
    game->maps[200][200]->x = 200;
    game->maps[200][200]->y = 200;
    generate(game->maps[200][200], -1, -1, -1, -1);

    while(!game->quit) {
        print_table(game->maps[game->current_y][game->current_x]->terrain);
        printf("(%d, %d)\n", game->current_x - 200, game->current_y - 200);

        scanf("%c", &c);
        menu(game, c);
    }
    teardown(game);
}