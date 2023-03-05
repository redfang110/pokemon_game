#ifndef POKE327_HPP
# define POKE327_HPP

# include <stdlib.h>
# include <assert.h>

#include "heap.h"
#include "character.hpp"
#include "enums.hpp"

class character_class;
class world_class;
class map_class;
class pc_class;

#define malloc(size) ({          \
  void *_tmp;                    \
  assert((_tmp = malloc(size))); \
  _tmp;                          \
})

/* Returns true if random float in [0,1] is less than *
 * numerator/denominator.  Uses only integer math.    */
# define rand_under(numerator, denominator) \
  (rand() < ((RAND_MAX / denominator) * numerator))

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((rand() % (((max) + 1) - (min))) + (min))

/* Even unallocated, a WORLD_SIZE x WORLD_SIZE array of pointers is a very *
 * large thing to put on the stack.  To avoid that, world is a global.     */
extern world_class world;

# define UNUSED(f) ((void) f)

typedef int16_t pair_t[num_dims];

#define MAP_X              80
#define MAP_Y              21
#define MIN_TREES          10
#define MIN_BOULDERS       10
#define TREE_PROB          95
#define BOULDER_PROB       95
#define WORLD_SIZE         401
#define MIN_TRAINERS       7   
#define ADD_TRAINER_PROB   50

#define mappair(pair) (m->map[pair[dim_y]][pair[dim_x]])
#define mapxy(x, y) (m->map[y][x])
#define heightpair(pair) (m->height[pair[dim_y]][pair[dim_x]])
#define heightxy(x, y) (m->height[y][x])

class map_class {
  public:
    map_class(heap_t turn, int32_t num_trainers, int8_t n, int8_t s, int8_t e, int8_t w)
    {
      this->turn = turn;
      this->num_trainers = num_trainers;
      this->n = n;
      this->s = s;
      this->e = e;
      this->w = w;
    }
    terrain_type map[MAP_Y][MAP_X];
    uint8_t height[MAP_Y][MAP_X];
    character_class *cmap[MAP_Y][MAP_X];
    heap_t turn;
    int32_t num_trainers;
    int8_t n, s, e, w;
};

class world_class {
  public:
    world_class() {}
    world_class(pair_t cur_idx, map_class *cur_map, character_class pc, int quit, int add_trainer_prob)
    {
      this->cur_idx[dim_x] = cur_idx[dim_x];
      this->cur_idx[dim_y] = cur_idx[dim_y];
      this->pc = pc;
      this->quit = quit;
      this->add_trainer_prob = add_trainer_prob;
    }
    map_class *world[WORLD_SIZE][WORLD_SIZE];
    pair_t cur_idx;
    map_class *cur_map;
    /* Please distance maps in world, not map, since *
     * we only need one pair at any given time.      */
    int hiker_dist[MAP_Y][MAP_X];
    int rival_dist[MAP_Y][MAP_X];
    character_class pc;
    int quit;
    int add_trainer_prob;
};

extern pair_t all_dirs[8];

#define rand_dir(dir) {     \
  int _i = rand() & 0x7;    \
  dir[0] = all_dirs[_i][0]; \
  dir[1] = all_dirs[_i][1]; \
}

struct path {
  heap_node_t *hn;
  uint8_t pos[2];
  uint8_t from[2];
  int32_t cost;
};

int new_map(int teleport);

#endif
