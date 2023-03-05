#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
// #include <vector>

#include "heap.h"
#include "poke327.h"
#include "io.h"

using std::cout;
using std::string;

typedef struct queue_node {
  int x, y;
  struct queue_node *next;
} queue_node_t;

world_t world;
pokedex_pokemon pokemon_list[1093];
pokedex_poke_moves poke_moves[528239];
pokedex_poke_species poke_species[899];
pokedex_poke_stats poke_stats[6553];
pokedex_poke_types poke_types[1676];
pokedex_experience experience[601];
pokedex_type_names type_names[19];
pokedex_moves moves[845];
pokedex_stats stats[9];

pair_t all_dirs[8] = {
  { -1, -1 },
  { -1,  0 },
  { -1,  1 },
  {  0, -1 },
  {  0,  1 },
  {  1, -1 },
  {  1,  0 },
  {  1,  1 },
};

static int32_t path_cmp(const void *key, const void *with) {
  return ((path_t *) key)->cost - ((path_t *) with)->cost;
}

static int32_t edge_penalty(int8_t x, int8_t y)
{
  return (x == 1 || y == 1 || x == MAP_X - 2 || y == MAP_Y - 2) ? 2 : 1;
}

static void dijkstra_path(map_t *m, pair_t from, pair_t to)
{
  static path_t path[MAP_Y][MAP_X], *p;
  static uint32_t initialized = 0;
  heap_t h;
  int32_t x, y;

  if (!initialized) {
    for (y = 0; y < MAP_Y; y++) {
      for (x = 0; x < MAP_X; x++) {
        path[y][x].pos[dim_y] = y;
        path[y][x].pos[dim_x] = x;
      }
    }
    initialized = 1;
  }
  
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      path[y][x].cost = INT_MAX;
    }
  }

  path[from[dim_y]][from[dim_x]].cost = 0;

  heap_init(&h, path_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      path[y][x].hn = heap_insert(&h, &path[y][x]);
    }
  }

  while ((p = (path_t *) heap_remove_min(&h))) {
    p->hn = NULL;

    if ((p->pos[dim_y] == to[dim_y]) && p->pos[dim_x] == to[dim_x]) {
      for (x = to[dim_x], y = to[dim_y];
           (x != from[dim_x]) || (y != from[dim_y]);
           p = &path[y][x], x = p->from[dim_x], y = p->from[dim_y]) {
        mapxy(x, y) = ter_path;
        heightxy(x, y) = 0;
      }
      heap_delete(&h);
      return;
    }

    if ((path[p->pos[dim_y] - 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1)))) {
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].cost =
        ((p->cost + heightpair(p->pos)) *
         edge_penalty(p->pos[dim_x], p->pos[dim_y] - 1));
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] - 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] - 1]
                                           [p->pos[dim_x]    ].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] - 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] - 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y])))) {
      path[p->pos[dim_y]][p->pos[dim_x] - 1].cost =
        ((p->cost + heightpair(p->pos)) *
         edge_penalty(p->pos[dim_x] - 1, p->pos[dim_y]));
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] - 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] - 1].hn);
    }
    if ((path[p->pos[dim_y]    ][p->pos[dim_x] + 1].hn) &&
        (path[p->pos[dim_y]    ][p->pos[dim_x] + 1].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y])))) {
      path[p->pos[dim_y]][p->pos[dim_x] + 1].cost =
        ((p->cost + heightpair(p->pos)) *
         edge_penalty(p->pos[dim_x] + 1, p->pos[dim_y]));
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y]    ][p->pos[dim_x] + 1].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y]    ]
                                           [p->pos[dim_x] + 1].hn);
    }
    if ((path[p->pos[dim_y] + 1][p->pos[dim_x]    ].hn) &&
        (path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost >
         ((p->cost + heightpair(p->pos)) *
          edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1)))) {
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].cost =
        ((p->cost + heightpair(p->pos)) *
         edge_penalty(p->pos[dim_x], p->pos[dim_y] + 1));
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_y] = p->pos[dim_y];
      path[p->pos[dim_y] + 1][p->pos[dim_x]    ].from[dim_x] = p->pos[dim_x];
      heap_decrease_key_no_replace(&h, path[p->pos[dim_y] + 1]
                                           [p->pos[dim_x]    ].hn);
    }
  }
}

static int build_paths(map_t *m)
{
  pair_t from, to;

  /*  printf("%d %d %d %d\n", m->n, m->s, m->e, m->w);*/

  if (m->e != -1 && m->w != -1) {
    from[dim_x] = 1;
    to[dim_x] = MAP_X - 2;
    from[dim_y] = m->w;
    to[dim_y] = m->e;

    dijkstra_path(m, from, to);
  }

  if (m->n != -1 && m->s != -1) {
    from[dim_y] = 1;
    to[dim_y] = MAP_Y - 2;
    from[dim_x] = m->n;
    to[dim_x] = m->s;

    dijkstra_path(m, from, to);
  }

  if (m->e == -1) {
    if (m->s == -1) {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    } else {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->w == -1) {
    if (m->s == -1) {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    } else {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->n == -1) {
    if (m->e == -1) {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    } else {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->s;
      to[dim_y] = MAP_Y - 2;
    }

    dijkstra_path(m, from, to);
  }

  if (m->s == -1) {
    if (m->e == -1) {
      from[dim_x] = 1;
      from[dim_y] = m->w;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    } else {
      from[dim_x] = MAP_X - 2;
      from[dim_y] = m->e;
      to[dim_x] = m->n;
      to[dim_y] = 1;
    }

    dijkstra_path(m, from, to);
  }

  return 0;
}

static int gaussian[5][5] = {
  {  1,  4,  7,  4,  1 },
  {  4, 16, 26, 16,  4 },
  {  7, 26, 41, 26,  7 },
  {  4, 16, 26, 16,  4 },
  {  1,  4,  7,  4,  1 }
};

static int smooth_height(map_t *m)
{
  int32_t i, x, y;
  int32_t s, t, p, q;
  queue_node_t *head, *tail, *tmp;
  /*  FILE *out;*/
  uint8_t height[MAP_Y][MAP_X];

  memset(&height, 0, sizeof (height));

  /* Seed with some values */
  for (i = 1; i < 255; i += 20) {
    do {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (height[y][x]);
    height[y][x] = i;
    if (i == 1) {
      head = tail = (queue_node_t *) malloc(sizeof (*tail));
    } else {
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);
  */
  
  /* Diffuse the vaules to fill the space */
  while (head) {
    x = head->x;
    y = head->y;
    i = height[y][x];

    if (x - 1 >= 0 && y - 1 >= 0 && !height[y - 1][x - 1]) {
      height[y - 1][x - 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y - 1;
    }
    if (x - 1 >= 0 && !height[y][x - 1]) {
      height[y][x - 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y;
    }
    if (x - 1 >= 0 && y + 1 < MAP_Y && !height[y + 1][x - 1]) {
      height[y + 1][x - 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x - 1;
      tail->y = y + 1;
    }
    if (y - 1 >= 0 && !height[y - 1][x]) {
      height[y - 1][x] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y - 1;
    }
    if (y + 1 < MAP_Y && !height[y + 1][x]) {
      height[y + 1][x] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x;
      tail->y = y + 1;
    }
    if (x + 1 < MAP_X && y - 1 >= 0 && !height[y - 1][x + 1]) {
      height[y - 1][x + 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y - 1;
    }
    if (x + 1 < MAP_X && !height[y][x + 1]) {
      height[y][x + 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y;
    }
    if (x + 1 < MAP_X && y + 1 < MAP_Y && !height[y + 1][x + 1]) {
      height[y + 1][x + 1] = i;
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
      tail->next = NULL;
      tail->x = x + 1;
      tail->y = y + 1;
    }

    tmp = head;
    head = head->next;
    free(tmp);
  }

  /* And smooth it a bit with a gaussian convolution */
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X) {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }
  /* Let's do it again, until it's smooth like Kenny G. */
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      for (s = t = p = 0; p < 5; p++) {
        for (q = 0; q < 5; q++) {
          if (y + (p - 2) >= 0 && y + (p - 2) < MAP_Y &&
              x + (q - 2) >= 0 && x + (q - 2) < MAP_X) {
            s += gaussian[p][q];
            t += height[y + (p - 2)][x + (q - 2)] * gaussian[p][q];
          }
        }
      }
      m->height[y][x] = t / s;
    }
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&height, sizeof (height), 1, out);
  fclose(out);

  out = fopen("smoothed.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->height, sizeof (m->height), 1, out);
  fclose(out);
  */

  return 0;
}

static void find_building_location(map_t *m, pair_t p)
{
  do {
    p[dim_x] = rand() % (MAP_X - 3) + 1;
    p[dim_y] = rand() % (MAP_Y - 3) + 1;

    if ((((mapxy(p[dim_x] - 1, p[dim_y]    ) == ter_path)     &&
          (mapxy(p[dim_x] - 1, p[dim_y] + 1) == ter_path))    ||
         ((mapxy(p[dim_x] + 2, p[dim_y]    ) == ter_path)     &&
          (mapxy(p[dim_x] + 2, p[dim_y] + 1) == ter_path))    ||
         ((mapxy(p[dim_x]    , p[dim_y] - 1) == ter_path)     &&
          (mapxy(p[dim_x] + 1, p[dim_y] - 1) == ter_path))    ||
         ((mapxy(p[dim_x]    , p[dim_y] + 2) == ter_path)     &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 2) == ter_path)))   &&
        (((mapxy(p[dim_x]    , p[dim_y]    ) != ter_mart)     &&
          (mapxy(p[dim_x]    , p[dim_y]    ) != ter_center)   &&
          (mapxy(p[dim_x] + 1, p[dim_y]    ) != ter_mart)     &&
          (mapxy(p[dim_x] + 1, p[dim_y]    ) != ter_center)   &&
          (mapxy(p[dim_x]    , p[dim_y] + 1) != ter_mart)     &&
          (mapxy(p[dim_x]    , p[dim_y] + 1) != ter_center)   &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_mart)     &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_center))) &&
        (((mapxy(p[dim_x]    , p[dim_y]    ) != ter_path)     &&
          (mapxy(p[dim_x] + 1, p[dim_y]    ) != ter_path)     &&
          (mapxy(p[dim_x]    , p[dim_y] + 1) != ter_path)     &&
          (mapxy(p[dim_x] + 1, p[dim_y] + 1) != ter_path)))) {
          break;
    }
  } while (1);
}

static int place_pokemart(map_t *m)
{
  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x]    , p[dim_y]    ) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y]    ) = ter_mart;
  mapxy(p[dim_x]    , p[dim_y] + 1) = ter_mart;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_mart;

  return 0;
}

static int place_center(map_t *m)
{  pair_t p;

  find_building_location(m, p);

  mapxy(p[dim_x]    , p[dim_y]    ) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y]    ) = ter_center;
  mapxy(p[dim_x]    , p[dim_y] + 1) = ter_center;
  mapxy(p[dim_x] + 1, p[dim_y] + 1) = ter_center;

  return 0;
}

static int map_terrain(map_t *m, int8_t n, int8_t s, int8_t e, int8_t w)
{
  int32_t i, x, y;
  queue_node_t *head, *tail, *tmp;
  //  FILE *out;
  int num_grass, num_clearing, num_mountain, num_forest, num_total;
  terrain_type_t type;
  int added_current = 0;
  
  num_grass = rand() % 4 + 2;
  num_clearing = rand() % 4 + 2;
  num_mountain = rand() % 2 + 1;
  num_forest = rand() % 2 + 1;
  num_total = num_grass + num_clearing + num_mountain + num_forest;

  memset(&m->map, 0, sizeof (m->map));

  /* Seed with some values */
  for (i = 0; i < num_total; i++) {
    do {
      x = rand() % MAP_X;
      y = rand() % MAP_Y;
    } while (m->map[y][x]);
    if (i == 0) {
      type = ter_grass;
    } else if (i == num_grass) {
      type = ter_clearing;
    } else if (i == num_grass + num_clearing) {
      type = ter_mountain;
    } else if (i == num_grass + num_clearing + num_mountain) {
      type = ter_forest;
    }
    m->map[y][x] = type;
    if (i == 0) {
      head = tail = (queue_node_t *) malloc(sizeof (*tail));
    } else {
      tail->next = (queue_node_t *) malloc(sizeof (*tail));
      tail = tail->next;
    }
    tail->next = NULL;
    tail->x = x;
    tail->y = y;
  }

  /*
  out = fopen("seeded.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */

  /* Diffuse the vaules to fill the space */
  while (head) {
    x = head->x;
    y = head->y;
    i = m->map[y][x];
    
    if (x - 1 >= 0 && !m->map[y][x - 1]) {
      if ((rand() % 100) < 80) {
        m->map[y][x - 1] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x - 1;
        tail->y = y;
      } else if (!added_current) {
        added_current = 1;
        m->map[y][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y - 1 >= 0 && !m->map[y - 1][x]) {
      if ((rand() % 100) < 20) {
        m->map[y - 1][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y - 1;
      } else if (!added_current) {
        added_current = 1;
        m->map[y][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (y + 1 < MAP_Y && !m->map[y + 1][x]) {
      if ((rand() % 100) < 20) {
        m->map[y + 1][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y + 1;
      } else if (!added_current) {
        added_current = 1;
        m->map[y][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    if (x + 1 < MAP_X && !m->map[y][x + 1]) {
      if ((rand() % 100) < 80) {
        m->map[y][x + 1] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x + 1;
        tail->y = y;
      } else if (!added_current) {
        added_current = 1;
        m->map[y][x] = (terrain_type_t) i;
        tail->next = (queue_node_t *) malloc(sizeof (*tail));
        tail = tail->next;
        tail->next = NULL;
        tail->x = x;
        tail->y = y;
      }
    }

    added_current = 0;
    tmp = head;
    head = head->next;
    free(tmp);
  }

  /*
  out = fopen("diffused.pgm", "w");
  fprintf(out, "P5\n%u %u\n255\n", MAP_X, MAP_Y);
  fwrite(&m->map, sizeof (m->map), 1, out);
  fclose(out);
  */
  
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (y == 0 || y == MAP_Y - 1 ||
          x == 0 || x == MAP_X - 1) {
        mapxy(x, y) = ter_boulder;
      }
    }
  }

  m->n = n;
  m->s = s;
  m->e = e;
  m->w = w;

  if (n != -1) {
    mapxy(n,         0        ) = ter_exit;
    mapxy(n,         1        ) = ter_path;
  }
  if (s != -1) {
    mapxy(s,         MAP_Y - 1) = ter_exit;
    mapxy(s,         MAP_Y - 2) = ter_path;
  }
  if (w != -1) {
    mapxy(0,         w        ) = ter_exit;
    mapxy(1,         w        ) = ter_path;
  }
  if (e != -1) {
    mapxy(MAP_X - 1, e        ) = ter_exit;
    mapxy(MAP_X - 2, e        ) = ter_path;
  }

  return 0;
}

static int place_boulders(map_t *m)
{
  int i;
  int x, y;

  for (i = 0; i < MIN_BOULDERS || rand() % 100 < BOULDER_PROB; i++) {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_forest && m->map[y][x] != ter_path) {
      m->map[y][x] = ter_boulder;
    }
  }

  return 0;
}

static int place_trees(map_t *m)
{
  int i;
  int x, y;
  
  for (i = 0; i < MIN_TREES || rand() % 100 < TREE_PROB; i++) {
    y = rand() % (MAP_Y - 2) + 1;
    x = rand() % (MAP_X - 2) + 1;
    if (m->map[y][x] != ter_mountain && m->map[y][x] != ter_path) {
      m->map[y][x] = ter_tree;
    }
  }

  return 0;
}

void rand_pos(pair_t pos)
{
  pos[dim_x] = (rand() % (MAP_X - 2)) + 1;
  pos[dim_y] = (rand() % (MAP_Y - 2)) + 1;
}

void new_hiker()
{
  pair_t pos;
  npc *c;

  do {
    rand_pos(pos);
  } while (world.hiker_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]         ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4            ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_hiker;
  c->mtype = move_hiker;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->defeated = 0;
  c->symbol = 'h';
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;

  //  printf("Hiker at %d,%d\n", pos[dim_x], pos[dim_y]);
}

void new_rival()
{
  pair_t pos;
  npc *c;

  do {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0        ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]         ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4            ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_rival;
  c->mtype = move_rival;
  c->dir[dim_x] = 0;
  c->dir[dim_y] = 0;
  c->defeated = 0;
  c->symbol = 'r';
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void new_char_other()
{
  pair_t pos;
  npc *c;

  do {
    rand_pos(pos);
  } while (world.rival_dist[pos[dim_y]][pos[dim_x]] == INT_MAX ||
           world.rival_dist[pos[dim_y]][pos[dim_x]] < 0        ||
           world.cur_map->cmap[pos[dim_y]][pos[dim_x]]         ||
           pos[dim_x] < 3 || pos[dim_x] > MAP_X - 4            ||
           pos[dim_y] < 3 || pos[dim_y] > MAP_Y - 4);

  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c = new npc;
  c->pos[dim_y] = pos[dim_y];
  c->pos[dim_x] = pos[dim_x];
  c->ctype = char_other;
  switch (rand() % 4) {
  case 0:
    c->mtype = move_pace;
    c->symbol = 'p';
    break;
  case 1:
    c->mtype = move_wander;
    c->symbol = 'w';
    break;
  case 2:
    c->mtype = move_sentry;
    c->symbol = 's';
    break;
  case 3:
    c->mtype = move_walk;
    c->symbol = 'n';
    break;
  }
  rand_dir(c->dir);
  c->defeated = 0;
  c->next_turn = 0;
  heap_insert(&world.cur_map->turn, c);
  world.cur_map->cmap[pos[dim_y]][pos[dim_x]] = c;
}

void place_characters()
{
  world.cur_map->num_trainers = 2;

  //Always place a hiker and a rival, then place a random number of others
  new_hiker();
  new_rival();
  do {
    //higher probability of non- hikers and rivals
    switch(rand() % 10) {
    case 0:
      new_hiker();
      break;
    case 1:
     new_rival();
      break;
    default:
      new_char_other();
      break;
    }
    /* Game attempts to continue to place trainers until the probability *
     * roll fails, but if the map is full (or almost full), it's         *
     * impossible (or very difficult) to continue to add, so we abort if *
     * we've tried MAX_TRAINER_TRIES times.                              */
  } while (++world.cur_map->num_trainers < MIN_TRAINERS ||
           ((rand() % 100) < ADD_TRAINER_PROB));
}

void init_pc()
{
  int x, y;

  do {
    x = rand() % (MAP_X - 2) + 1;
    y = rand() % (MAP_Y - 2) + 1;
  } while (world.cur_map->map[y][x] != ter_path);

  world.pc.pos[dim_x] = x;
  world.pc.pos[dim_y] = y;
  world.pc.symbol = '@';

  world.cur_map->cmap[y][x] = &world.pc;
  world.pc.next_turn = 0;

  heap_insert(&world.cur_map->turn, &world.pc);
}

void place_pc()
{
  character *c;

  if (world.pc.pos[dim_x] == 1) {
    world.pc.pos[dim_x] = MAP_X - 2;
  } else if (world.pc.pos[dim_x] == MAP_X - 2) {
    world.pc.pos[dim_x] = 1;
  } else if (world.pc.pos[dim_y] == 1) {
    world.pc.pos[dim_y] = MAP_Y - 2;
  } else if (world.pc.pos[dim_y] == MAP_Y - 2) {
    world.pc.pos[dim_y] = 1;
  }

  world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = &world.pc;

  if ((c = (character *) heap_peek_min(&world.cur_map->turn))) {
    world.pc.next_turn = c->next_turn;
  } else {
    world.pc.next_turn = 0;
  }
}

// New map expects cur_idx to refer to the index to be generated.  If that
// map has already been generated then the only thing this does is set
// cur_map.
int new_map(int teleport)
{
  int d, p;
  int e, w, n, s;
  int x, y;
  
  if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]]) {
    world.cur_map = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]];
    place_pc();

    return 0;
  }

  world.cur_map                                             =
    world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x]] =
    (map_t *) malloc(sizeof (*world.cur_map));

  smooth_height(world.cur_map);
  
  if (!world.cur_idx[dim_y]) {
    n = -1;
  } else if (world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]]) {
    n = world.world[world.cur_idx[dim_y] - 1][world.cur_idx[dim_x]]->s;
  } else {
    n = 3 + rand() % (MAP_X - 6);
  }
  if (world.cur_idx[dim_y] == WORLD_SIZE - 1) {
    s = -1;
  } else if (world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]]) {
    s = world.world[world.cur_idx[dim_y] + 1][world.cur_idx[dim_x]]->n;
  } else  {
    s = 3 + rand() % (MAP_X - 6);
  }
  if (!world.cur_idx[dim_x]) {
    w = -1;
  } else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1]) {
    w = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] - 1]->e;
  } else {
    w = 3 + rand() % (MAP_Y - 6);
  }
  if (world.cur_idx[dim_x] == WORLD_SIZE - 1) {
    e = -1;
  } else if (world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1]) {
    e = world.world[world.cur_idx[dim_y]][world.cur_idx[dim_x] + 1]->w;
  } else {
    e = 3 + rand() % (MAP_Y - 6);
  }
  
  map_terrain(world.cur_map, n, s, e, w);
     
  place_boulders(world.cur_map);
  place_trees(world.cur_map);
  build_paths(world.cur_map);
  d = (abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)) +
       abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)));
  p = d > 200 ? 5 : (50 - ((45 * d) / 200));
  //  printf("d=%d, p=%d\n", d, p);
  if ((rand() % 100) < p || !d) {
    place_pokemart(world.cur_map);
  }
  if ((rand() % 100) < p || !d) {
    place_center(world.cur_map);
  }

  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      world.cur_map->cmap[y][x] = NULL;
    }
  }

  heap_init(&world.cur_map->turn, cmp_char_turns, delete_character);

  if ((world.cur_idx[dim_x] == WORLD_SIZE / 2) &&
      (world.cur_idx[dim_y] == WORLD_SIZE / 2)) {
    init_pc();
  } else {
    place_pc();
  }

  if (teleport) {
    do {
      world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = NULL;
      world.pc.pos[dim_x] = rand_range(1, MAP_X - 2);
      world.pc.pos[dim_y] = rand_range(1, MAP_Y - 2);
    } while (world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] ||
             (move_cost[char_pc][world.cur_map->map[world.pc.pos[dim_y]]
                                                   [world.pc.pos[dim_x]]] ==
              INT_MAX)                                                      ||
             world.rival_dist[world.pc.pos[dim_y]][world.pc.pos[dim_x]] < 0);
    world.cur_map->cmap[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = &world.pc;
  }

  pathfind(world.cur_map);
  
  place_characters();

  return 0;
}

/*
static void print_map()
{
  int x, y;
  int default_reached = 0;
  printf("\n\n\n");
  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.cur_map->cmap[y][x]) {
        putchar(world.cur_map->cmap[y][x]->symbol);
      } else {
        switch (world.cur_map->map[y][x]) {
        case ter_boulder:
        case ter_mountain:
          putchar('%');
          break;
        case ter_tree:
        case ter_forest:
          putchar('^');
          break;
        case ter_path:
          putchar('#');
          break;
        case ter_mart:
          putchar('M');
          break;
        case ter_center:
          putchar('C');
          break;
        case ter_grass:
          putchar(':');
          break;
        case ter_clearing:
          putchar('.');
          break;
        default:
          default_reached = 1;
          break;
        }
      }
    }
    putchar('\n');
  }
  if (default_reached) {
    fprintf(stderr, "Default reached in %s\n", __FUNCTION__);
  }
}
*/

// The world is global because of its size, so init_world is parameterless
void init_world()
{
  world.quit = 0;
  world.cur_idx[dim_x] = world.cur_idx[dim_y] = WORLD_SIZE / 2;
  new_map(0);
}

void delete_world()
{
  int x, y;

  //Only correct because current game never leaves the initial map
  //Need to iterate over all maps in 1.05+
  heap_delete(&world.cur_map->turn);

  for (y = 0; y < WORLD_SIZE; y++) {
    for (x = 0; x < WORLD_SIZE; x++) {
      if (world.world[y][x]) {
        free(world.world[y][x]);
        world.world[y][x] = NULL;
      }
    }
  }
}

void print_hiker_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.hiker_dist[y][x] == INT_MAX) {
        printf("   ");
      } else {
        printf(" %5d", world.hiker_dist[y][x]);
      }
    }
    printf("\n");
  }
}

void print_rival_dist()
{
  int x, y;

  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      if (world.rival_dist[y][x] == INT_MAX || world.rival_dist[y][x] < 0) {
        printf("   ");
      } else {
        printf(" %02d", world.rival_dist[y][x] % 100);
      }
    }
    printf("\n");
  }
}

void leave_map(pair_t d)
{
  if (d[dim_x] == 0) {
    world.cur_idx[dim_x]--;
  } else if (d[dim_y] == 0) {
    world.cur_idx[dim_y]--;
  } else if (d[dim_x] == MAP_X - 1) {
    world.cur_idx[dim_x]++;
  } else {
    world.cur_idx[dim_y]++;
  }
  new_map(0);
}

void game_loop()
{
  character *c;
  pair_t d;
  bool is_pc;

  while (!world.quit) {
    c = (character *) heap_remove_min(&world.cur_map->turn);
    is_pc = dynamic_cast<npc *>(c) == NULL;

    move_func[is_pc ? move_pc : ((npc *) c)->mtype](c, d);

    world.cur_map->cmap[c->pos[dim_y]][c->pos[dim_x]] = NULL;
    if (is_pc && (d[dim_x] == 0 || d[dim_x] == MAP_X - 1 ||
                  d[dim_y] == 0 || d[dim_y] == MAP_Y - 1)) {
      leave_map(d);
      d[dim_x] = c->pos[dim_x];
      d[dim_y] = c->pos[dim_y];
    }
    world.cur_map->cmap[d[dim_y]][d[dim_x]] = c;

    if (is_pc) {
      pathfind(world.cur_map);
    }

    c->next_turn += move_cost[is_pc ? char_pc : ((npc *) c)->ctype]
                             [world.cur_map->map[d[dim_y]][d[dim_x]]];

    c->pos[dim_y] = d[dim_y];
    c->pos[dim_x] = d[dim_x];

    heap_insert(&world.cur_map->turn, c);
  }
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s [-s|--seed <seed>]\n", s);

  exit(1);
}

// pokedex_pokemon pokemon_list[1093];
// pokedex_poke_moves poke_moves[528239];
// pokedex_poke_species poke_species[899];
// pokedex_poke_stats poke_stats[6553];
// pokedex_poke_types poke_types[1676];
// pokedex_experience experience[601];
// pokedex_type_names type_names[19];
// pokedex_moves moves[845];
// pokedex_stats stats[9];

std::vector<pokedex_moves> init_poke_moves(Pokemon poke)
{
  std::vector<pokedex_moves> moves_ids;
  for (int i = 1; i < 528239; i++) {
    if (poke_moves[i].pokemon_id == pokemon_list[poke.id].species_id && poke_moves[poke.id].pokemon_move_method_id == 1) {
      if (poke_moves[i].level <= poke.level) {
        moves_ids.push_back(moves[poke_moves[i].move_id]);
      }
    }
  }

  if (!moves_ids.empty()) {
    while (moves_ids.size() > 2) {
      moves_ids.erase(moves_ids.begin() + (rand() % moves_ids.size()));
    }
  } else {
    while (moves_ids.size() < 1) {
      poke.level++;
      for (int i = 1; i < 528239; i++) {
        if (poke_moves[i].pokemon_id == pokemon_list[poke.id].species_id && poke_moves[poke.id].pokemon_move_method_id == 1) {
          if (poke_moves[i].level <= poke.level) {
            moves_ids.push_back(moves[poke_moves[i].move_id]);
          }
        }
        if (moves_ids.size() == 2) {
          break;
        }
      }
    }
  }
  return moves_ids;
}

/*  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* To find the level-up moveset of a pokemon, you’ll want ´ pokemon_moves.pokemon_id equal to  *
* pokemon.species_id and pokemon_moves.pokemon_move_method_id equal to 1. From these,         *
* select your two moves from a uniform distribution.                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
Pokemon::Pokemon(int level) {
  //initialize id, level, identifier, species_id, and base_experience
  int id = (rand() % 1092) + 1;
  this->level = level;
  identifier = pokemon_list[id].identifier;
  species_id = pokemon_list[id].species_id;
  base_experience = pokemon_list[id].base_experience;

  //initialize gender
  if (rand() % 2 == 0) {
    gender = male;
  } else {
    gender = female;
  }

  //initialize is_shiny
  if (rand() % 8192 == 0) {
    is_shiny = true;
  } else {
    is_shiny = false;
  }

  //initialize movesList TODO
  movesList = init_poke_moves(*this);
  
  //initialize IVs
  IVs.HP = rand() % 16;
  IVs.attack = rand() % 16;
  IVs.defense = rand() % 16;
  IVs.speed = rand() % 16;
  IVs.special_attack = rand() % 16;
  IVs.special_defense = rand() % 16;

  //initialize base_stats
  for (int i = 1; i < 6553; i++) {
    if (poke_stats[i].pokemon_id == id) {
      switch (poke_stats[i].stat_id)
      {
      case HP:
        base_stats.HP = poke_stats[i].base_stat;
        break;
      case attack:
        base_stats.attack = poke_stats[i].base_stat;
        break;
      case defense:
        base_stats.defense = poke_stats[i].base_stat;
        break;
      case special_attack:
        base_stats.special_attack = poke_stats[i].base_stat;
        break;
      case special_defense:
        base_stats.special_defense = poke_stats[i].base_stat;
        break;
      case speed:
        base_stats.speed = poke_stats[i].base_stat;
        break;
      default:
        cout << "Pokemon constructor default reached\n";
        break;
      }
    }
  }
  //initialize stats
  stats.HP = (((base_stats.HP + IVs.HP) * 2 * level) / 100) + level + 10;
  stats.attack = (((base_stats.attack + IVs.attack) * 2 * level) / 100) + 5;
  stats.defense = (((base_stats.defense + IVs.defense) * 2 * level) / 100) + 5;
  stats.special_attack = (((base_stats.special_attack + IVs.special_attack) * 2 * level) / 100) + 5;
  stats.special_defense = (((base_stats.special_defense + IVs.special_defense) * 2 * level) / 100) + 5;
  stats.speed = (((base_stats.speed + IVs.speed) * 2 * level) / 100) + 5;
}

void initFiles()
{
  for (int i = 0; i < 1093; i++)
  {
    pokemon_list[i] = pokedex_pokemon();
  }

  for (int i = 0; i < 528239; i++)
  {
    poke_moves[i] = pokedex_poke_moves();
  }

  for (int i = 0; i < 899; i++)
  {
    poke_species[i] = pokedex_poke_species();
  }

  for (int i = 0; i < 6553; i++)
  {
    poke_stats[i] = pokedex_poke_stats();
  }

  for (int i = 0; i < 1676; i++)
  {
    poke_types[i] = pokedex_poke_types();
  }

  for (int i = 0; i < 601; i++)
  {
    experience[i] = pokedex_experience();
  }

  for (int i = 0; i < 19; i++)
  {
    type_names[i] = pokedex_type_names();
  }

  for (int i = 0; i < 845; i++)
  {
    moves[i] = pokedex_moves();
  }
  for (int i = 0; i < 9; i++)
  {
    stats[i] = pokedex_stats();
  }
}

std::string getBase()
{
  struct stat buffer;
  string base;
  char* check;
  base = getenv("HOME");
  base += "/.poke327/pokedex/pokedex/data/csv/";
  check = (char *) malloc(base.size());
  check = (char *) base.c_str();

  if (stat(check, &buffer)) {
    // free((void *) check);
    check = NULL;
  }

  if(!check) {
    if (!stat("/share/cs327", &buffer)) {
      base = "/share/cs327/pokedex/pokedex/data/csv/";
    } else {
      return "";
    }
  }
  // free((void *) check);

  return base;
}

int getNext(string str, string deliminator)
{
  int num;
  // string temp = "";
  std::stringstream ss;
  // cout << str << "     ";
  if (str == "") {
    // cout << "int max\n";
    return INT_MAX;
  }
  int i = 0;
  while (i != str.size() && str[i] != deliminator[0]) {
    // temp += str[i]; 
    i++;
    // if (i == str.size()) {
    //   break;
    // }
  }
  ss << str.substr(0, i);
  ss >> num;
  // cout << num << std::endl;
  return num;
}

int parseAll()
{
  // cout << "parse start\n";
  initFiles();
  int success = 0;
  std::ifstream f;
  string input, file, base = getBase(), deliminator = ",";
  if (base == "")
  {
    cout << "parse error\n";
    return 1;
  }

  // cout << "parse pokemon\n";
  file = base + "pokemon.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "pokemon error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 1093; i++) {
    std::getline(f, input);
    string str;
    int temp, pos1 = 0, pos2;
    // cout << input << std::endl;

    pos2 = input.find(deliminator);
    // cout << "parse pokemon pre getNext\n";
    temp = getNext(input.substr(pos1, pos2), ",");
    // cout << "parse pokemon post getNext\n";
    pokemon_list[i].id = temp;
    // cout << "parse pokemon post assign\n";
    // cout << temp << std::endl;
    // cout << "parse pokemon post cout temp\n";

    // cout << "parse pokemon pre substring\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    // cout << "parse pokemon pre find\n";
    pos2 = input.find(deliminator);
    // cout << "parse pokemon pre getNext\n";
    str = input.substr(pos1, pos2 + 1);
    // cout << "parse pokemon pre getNext\n";
    pokemon_list[i].identifier = str;
    // cout << str << std::endl;

    // cout << "parse pokemon prespecies_id\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    pokemon_list[i].species_id = temp;
    // cout << temp << std::endl;

    // cout << "parse pokemon pre height\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    pokemon_list[i].height = temp;
    // cout << temp << std::endl;

    // cout << "parse pokemon pre weight\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    // cout << "parse pokemon pre find\n";
    pos2 = input.find(deliminator);
    // cout << "parse pokemon pre getNext\n";
    temp = getNext(input.substr(pos1, pos2), ",");
    // cout << "parse pokemon pre weight = temp\n";
    pokemon_list[i].weight = temp;
    // cout << "parse pokemon pre print temp\n";
    // cout << temp << std::endl;

    // cout << "parse pokemon pre base_experience\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    pokemon_list[i].base_experience = temp;
    // cout << temp << std::endl;

    // cout << "parse pokemon pre order\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    pokemon_list[i].order = temp;
    // cout << temp << std::endl;

    // cout << "parse pokemon pre is_default\n";
    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    pokemon_list[i].is_default = temp;
    // cout << temp << std::endl;
  }
  f.close();

  // cout << "parse pokemon_moves\n";
  file = base + "pokemon_moves.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "pokemon_move error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 528239; i++) {
    std::getline(f, input);
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].pokemon_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].version_group_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].move_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].pokemon_move_method_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].level = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_moves[i].order = temp;
  }
  f.close();

  // cout << "parse pokemon_species\n";
  file = base + "pokemon_species.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "pokemon_species error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 899; i++) {
    std::getline(f, input);
    string str;
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    str = input.substr(pos1, pos2);
    poke_species[i].identifier = str;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].generation_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].evolves_from_species_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].evolution_chain_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].color_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].shape_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].habitat_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].gender_rate = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].capture_rate = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].capture_rate = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].base_happiness = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].is_baby = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].hatch_counter = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].has_gender_differences = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].growth_rate_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].forms_switchable = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].is_legendary = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].is_mythical = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].order = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_species[i].conquest_order = temp;
  }
  f.close();

  // cout << "parse pokemon_stats\n";
  file = base + "pokemon_stats.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "pokemon_stats error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 6553; i++) {
    std::getline(f, input);
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_stats[i].pokemon_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_stats[i].stat_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_stats[i].base_stat = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_stats[i].effort = temp;
  }
  f.close();

  // cout << "parse pokemon_types\n";
  file = base + "pokemon_types.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "pokemon_types error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 1676; i++) {
    std::getline(f, input);
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_types[i].pokemon_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_types[i].type_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    poke_types[i].slot = temp;
  }
  f.close();

  // cout << "parse experience\n";
  file = base + "experience.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "experience error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 601; i++) {
    std::getline(f, input);
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    experience[i].growth_rate_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    experience[i].level = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    experience[i].experience = temp;
  }
  f.close();

  // cout << "parse type_names\n";
  file = base + "type_names.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "type_names error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 19; i++) {
    std::getline(f, input);
    string str;
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    type_names[i].type_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    type_names[i].local_language_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    str = input.substr(pos1, pos2);
    type_names[i].name = str;

    if (type_names[i].local_language_id != 9) {
      i--;
    }
  }
  f.close();

  
  // cout << "parse moves\n";
  file = base + "moves.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "moves error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 845; i++) {
    std::getline(f, input);
    string str;
    int temp, pos1 = 0, pos2;
    // cout << input << std::endl;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    str = input.substr(pos1, pos2);
    moves[i].identifier = str;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].generation_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].type_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].power = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].pp = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].accuracy = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].priority = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].target_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].damage_class_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].effect_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].effect_chance = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].contest_type_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].contest_effect_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    moves[i].super_contest_effect_id = temp;
  }
  f.close();

  // cout << "parse stats\n";
  file = base + "stats.csv";
  f.open(file);
  if (!f.is_open()) {
    success = 2;
    cout << "stats error\n";
  }

  std::getline(f, input);
  for (int i = 1; i < 845; i++) {
    std::getline(f, input);
    string str;
    int temp, pos1 = 0, pos2;

    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    stats[i].id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    stats[i].damage_class_id = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    str = input.substr(pos1, pos2);
    stats[i].identifier = str;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    stats[i].is_battle_only = temp;

    input = input.substr(pos2 + 1, input.size() - 1);
    pos2 = input.find(deliminator);
    temp = getNext(input.substr(pos1, pos2), ",");
    stats[i].game_index = temp;
  }
  f.close();

  // sleep(1);

  return success;
}

string printNext(int i)
{
  if (i == INT_MAX)
  {
    return "";
  }
  return std::to_string(i);
}

void printFile(char *argv[])
{
  string arg = argv[1];

  if (arg == "pokemon")
  {
    cout << "id,identifier,species_id,height,weight,base_experience,oder,is_default\n";
    for (int i = 1; i <= 1092; i++)
    {
      cout << printNext(pokemon_list[i].id) << ",";
      cout << pokemon_list[i].identifier << ",";
      cout << printNext(pokemon_list[i].species_id) << ",";
      cout << printNext(pokemon_list[i].height) << ",";
      cout << printNext(pokemon_list[i].weight) << ",";
      cout << printNext(pokemon_list[i].base_experience) << ",";
      cout << printNext(pokemon_list[i].order) << ",";
      cout << printNext(pokemon_list[i].is_default) << "\n";
    }
  }

  if (arg == "moves")
  {
    cout << "id,identifier,generation_id,type_id,power,pp,accuracy,priority,target_id,damage_class_id,effect_id,effect_chance;,contest_type_id,contest_effect_id,super_contest_effect_id\n";
    for (int i = 1; i <= 844; i++)
    {
      cout << printNext(moves[i].id) << ",";
      cout << moves[i].identifier << ",";
      cout << printNext(moves[i].generation_id) << ",";
      cout << printNext(moves[i].type_id) << ",";
      cout << printNext(moves[i].power) << ",";
      cout << printNext(moves[i].pp) << ",";
      cout << printNext(moves[i].accuracy) << ",";
      cout << printNext(moves[i].priority) << ",";
      cout << printNext(moves[i].target_id) << ",";
      cout << printNext(moves[i].damage_class_id) << ",";
      cout << printNext(moves[i].effect_id) << ",";
      cout << printNext(moves[i].effect_chance) << ",";
      cout << printNext(moves[i].contest_type_id) << ",";
      cout << printNext(moves[i].contest_effect_id) << ",";
      cout << printNext(moves[i].super_contest_effect_id) << "\n";
    }
  }

  if (arg == "pokemon_moves") {
    cout << "pokemon_id,version_group_id,move_id,pokemon_move_method_id,level,order\n";
    for (int i = 1; i <= 528238; i++)
    {
      cout << printNext(poke_moves[i].pokemon_id) << ",";
      cout << printNext(poke_moves[i].version_group_id) << ",";
      cout << printNext(poke_moves[i].move_id) << ",";
      cout << printNext(poke_moves[i].pokemon_move_method_id) << ",";
      cout << printNext(poke_moves[i].level) << ",";
      cout << printNext(poke_moves[i].order) << "\n";
    }
  }

  if (arg == "pokemon_species") {
    cout << "id,identifier,generation_id,evolves_from_species_id,evolution_chain_id,color_id,shape_id,habitat_id,gender_rate,capture_rate,base_happiness,is_baby,hatch_counter,has_gender_differences,growth_rate_id,forms_switchable,is_legendary,is_mythical,order,conquest_order\n";
    for (int i = 1; i <= 898; i++)
    {
      cout << printNext(poke_species[i].id) << ",";
      cout << poke_species[i].identifier << ",";
      cout << printNext(poke_species[i].generation_id) << ",";
      cout << printNext(poke_species[i].evolves_from_species_id) << ",";
      cout << printNext(poke_species[i].evolution_chain_id) << ",";
      cout << printNext(poke_species[i].color_id) << ",";
      cout << printNext(poke_species[i].shape_id) << ",";
      cout << printNext(poke_species[i].habitat_id) << ",";
      cout << printNext(poke_species[i].gender_rate) << ",";
      cout << printNext(poke_species[i].capture_rate) << ",";
      cout << printNext(poke_species[i].base_happiness) << ",";
      cout << printNext(poke_species[i].is_baby) << ",";
      cout << printNext(poke_species[i].hatch_counter) << ",";
      cout << printNext(poke_species[i].has_gender_differences) << ",";
      cout << printNext(poke_species[i].growth_rate_id) << ",";
      cout << printNext(poke_species[i].forms_switchable) << ",";
      cout << printNext(poke_species[i].is_legendary) << ",";
      cout << printNext(poke_species[i].is_mythical) << ",";
      cout << printNext(poke_species[i].order) << ",";
      cout << printNext(poke_species[i].conquest_order) << "\n";
    }
  }

  if (arg == "experience") {
    cout << "growth_rate_id,level,experience\n";
    for (int i = 1; i <= 600; i++)
    {
      cout << printNext(experience[i].growth_rate_id) << ",";
      cout << printNext(experience[i].level) << ",";
      cout << printNext(experience[i].experience) << "\n";
    }
  }

  if (arg == "type_names") {
    cout << "type_id,local_language_id,name\n";
    for (int i = 1; i <= 18; i++)
    {
      cout << printNext(type_names[i].type_id) << ",";
      cout << printNext(type_names[i].local_language_id) << ",";
      cout << type_names[i].name << "\n";
    }
  }

  if (arg == "pokemon_stats") {
    cout << "pokemon_id,stat_id,base_stat,effort\n";
    for (int i = 1; i <= 6552; i++)
    {
      cout << printNext(poke_stats[i].pokemon_id) << ",";
      cout << printNext(poke_stats[i].stat_id) << ",";
      cout << printNext(poke_stats[i].base_stat) << ",";
      cout << printNext(poke_stats[i].effort) << "\n";
    }
  }

  if (arg == "stats") {
    cout << "id,damage_class_id,identifier,is_battle_only,game_index\n";
    for (int i = 1; i <= 8; i++)
    {
      cout << printNext(stats[i].id) << ",";
      cout << printNext(stats[i].damage_class_id) << ",";
      cout << stats[i].identifier << ",";
      cout << printNext(stats[i].is_battle_only) << ",";
      cout << printNext(stats[i].game_index) << "\n";
    }
  }

  if (arg == "pokemon_types") {
    cout << "pokemon_id,type_id,slot\n";
    for (int i = 1; i <= 1675; i++)
    {
      cout << printNext(poke_types[i].pokemon_id) << ",";
      cout << printNext(poke_types[i].type_id) << ",";
      cout << printNext(poke_types[i].slot) << "\n";
    }
  }
}

int main(int argc, char *argv[])
{
  cout << "Start\n";
  if (parseAll() != 0)
  {
    cout << "Some file could not be read..\n";
  }
  cout << "parsed\n";
  sleep(1);

  // printFile(argv);
  // cout << "done\n";

  // pokemon, 
  // moves, 
  // pokemon moves,
  // pokemon species, 
  // experience, 
  // type names, 
  // pokemon stats, 
  // stats 
  // pokemon types

  // std::string file = base + "pokedex/data/csv/pokemon_types.csv";
  // ifstream f(file); wont work, need an import?
  // file = base + “pokedex/data/csv/pokemon.csv”;
  // f.open(file);
  // return 0;
  struct timeval tv;
  uint32_t seed;
  int long_arg;
  int do_seed;
  //  char c;
  //  int x, y;
  int i;

  do_seed = 1;
  
  cout << "check args\n";
  sleep(1);
  if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-seed")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%u", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }
  cout << "args checked\n";
  sleep(1);

  if (do_seed) {
    /* Allows me to start the game more than once *
     * per second, as opposed to time().          */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }
  cout << "seed done\n";
  sleep(1);

  printf("Using seed: %u\n", seed);
  srand(seed);

  cout << "init terminal\n";
  sleep(1);
  io_init_terminal();
  cout << "init world\n";
  sleep(1);
  init_world();

  /* print_hiker_dist(); */
  
  /*
  do {
    print_map();  
    printf("Current position is %d%cx%d%c (%d,%d).  "
           "Enter command: ",
           abs(world.cur_idx[dim_x] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_x] - (WORLD_SIZE / 2) >= 0 ? 'E' : 'W',
           abs(world.cur_idx[dim_y] - (WORLD_SIZE / 2)),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2) <= 0 ? 'N' : 'S',
           world.cur_idx[dim_x] - (WORLD_SIZE / 2),
           world.cur_idx[dim_y] - (WORLD_SIZE / 2));
    scanf(" %c", &c);
    switch (c) {
    case 'n':
      if (world.cur_idx[dim_y]) {
        world.cur_idx[dim_y]--;
        new_map();
      }
      break;
    case 's':
      if (world.cur_idx[dim_y] < WORLD_SIZE - 1) {
        world.cur_idx[dim_y]++;
        new_map();
      }
      break;
    case 'e':
      if (world.cur_idx[dim_x] < WORLD_SIZE - 1) {
        world.cur_idx[dim_x]++;
        new_map();
      }
      break;
    case 'w':
      if (world.cur_idx[dim_x]) {
        world.cur_idx[dim_x]--;
        new_map();
      }
      break;
     case 'q':
      break;
    case 'f':
      scanf(" %d %d", &x, &y);
      if (x >= -(WORLD_SIZE / 2) && x <= WORLD_SIZE / 2 &&
          y >= -(WORLD_SIZE / 2) && y <= WORLD_SIZE / 2) {
        world.cur_idx[dim_x] = x + (WORLD_SIZE / 2);
        world.cur_idx[dim_y] = y + (WORLD_SIZE / 2);
        new_map();
      }
      break;
    case '?':
    case 'h':
      printf("Move with 'e'ast, 'w'est, 'n'orth, 's'outh or 'f'ly x y.\n"
             "Quit with 'q'.  '?' and 'h' print this help message.\n");
      break;
    default:
      fprintf(stderr, "%c: Invalid input.  Enter '?' for help.\n", c);
      break;
    }
  } while (c != 'q');

  */

  cout << "game loop starting\n";
  sleep(1);
  game_loop();
  cout << "delete world\n";
  sleep(1);
  delete_world();
  cout << "terminal reset\n";
  sleep(1);
  io_reset_terminal();
  
  return 0;
}
