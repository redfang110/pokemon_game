#include <limits.h>

#include "character.hpp"
#include "poke327.hpp"
#include "io.hpp"

/***********************************************************************
 * Hack: Avoid the "path to a building" issue by making building cells *
 * have a large but not infinite movement cost.  This allows the       *
 * pathfinding algorithm to run without overflowing, but still makes   *
 * NPCs avoid buildings most of the time since the cost to move on to  *
 * the building is greater than navigating the building's perimeter.   *
 ***********************************************************************/
int32_t move_cost[num_character_types][num_terrain_types] = {
  { INT_MAX, INT_MAX, 10, 10, 10, 20, 10, INT_MAX, INT_MAX, 10      },
  { INT_MAX, INT_MAX, 10, 50, 50, 15, 10, 15,      15,      INT_MAX },
  { INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX },
  { INT_MAX, INT_MAX, 10, 50, 50, 20, 10, INT_MAX, INT_MAX, INT_MAX },
};

const char *char_type_name[num_character_types] = {
  "PC",
  "Hiker",
  "Rival",
  "Trainer",
};

// void character_class::copy(character_class *npc) {
//   this->pos[dim_x] = npc->pos[dim_x];
//   this->pos[dim_y] = npc->pos[dim_y];
//   this->symbol = npc->symbol;
//   this->next_turn = npc->next_turn;
//   this->ctype = npc->ctype;
//   this->mtype = npc->mtype;
//   this->dir[dim_x] = npc->dir[dim_x];
//   this->dir[dim_y] = npc->dir[dim_y];
//   this->defeated = npc->defeated;
// }

// void character_class::move(pair_t& dest) {
//   //do nothing
// }

// void hiker_class::move(pair_t& dest)
// {
//   int min;
//   int base;
//   int i;
//   base = rand() & 0x7;
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
//   min = INT_MAX;
//   for (i = base; i < 8 + base; i++) {
//     if ((world.hiker_dist[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                          [pos[dim_x] + all_dirs[i & 0x7][dim_x]] <=
//          min) &&
//         !world.cur_map->cmap[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                             [pos[dim_x] + all_dirs[i & 0x7][dim_x]]) {
//       dest[dim_x] = pos[dim_x] + all_dirs[i & 0x7][dim_x];
//       dest[dim_y] = pos[dim_y] + all_dirs[i & 0x7][dim_y];
//       min = world.hiker_dist[dest[dim_y]][dest[dim_x]];
//     }
//     if (world.hiker_dist[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                         [pos[dim_x] + all_dirs[i & 0x7][dim_x]] == 0) {
//       io_battle(this, &world.pc);
//       break;
//     }
//   }
// }

// void rival_class::move(pair_t& dest)
// {
//   int min;
//   int base;
//   int i;
//   base = rand() & 0x7;
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
//   min = INT_MAX;
//   for (i = base; i < 8 + base; i++) {
//     if ((world.rival_dist[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                          [pos[dim_x] + all_dirs[i & 0x7][dim_x]] <
//          min) &&
//         !world.cur_map->cmap[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                             [pos[dim_x] + all_dirs[i & 0x7][dim_x]]) {
//       dest[dim_x] = pos[dim_x] + all_dirs[i & 0x7][dim_x];
//       dest[dim_y] = pos[dim_y] + all_dirs[i & 0x7][dim_y];
//       min = world.rival_dist[dest[dim_y]][dest[dim_x]];
//     }
//     if (world.rival_dist[pos[dim_y] + all_dirs[i & 0x7][dim_y]]
//                         [pos[dim_x] + all_dirs[i & 0x7][dim_x]] == 0) {
//       io_battle(this, &world.pc);
//       break;
//     }
//   }
// }

// void pacer_class::move(pair_t& dest)
// {
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
//   if (!defeated &&
//       world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] ==
//       &world.pc) {
//       io_battle(this, &world.pc);
//       return;
//   }
//   if ((world.cur_map->map[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] !=
//        world.cur_map->map[pos[dim_y]][pos[dim_x]]) ||
//       world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]]) {
//     dir[dim_x] *= -1;
//     dir[dim_y] *= -1;
//   }
//   if ((world.cur_map->map[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] ==
//        world.cur_map->map[pos[dim_y]][pos[dim_x]]) &&
//       !world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                           [pos[dim_x] + dir[dim_x]]) {
//     dest[dim_x] = pos[dim_x] + dir[dim_x];
//     dest[dim_y] = pos[dim_y] + dir[dim_y];
//   }
// }

// void wanderer_class::move(pair_t& dest)
// {
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
//   if (!defeated &&
//       world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] ==
//       &world.pc) {
//       io_battle(this, &world.pc);
//       return;
//   }
//   if ((world.cur_map->map[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] !=
//        world.cur_map->map[pos[dim_y]][pos[dim_x]]) ||
//       world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]]) {
//     rand_dir(dir);
//   }
//   if ((world.cur_map->map[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] ==
//        world.cur_map->map[pos[dim_y]][pos[dim_x]]) &&
//       !world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                           [pos[dim_x] + dir[dim_x]]) {
//     dest[dim_x] = pos[dim_x] + dir[dim_x];
//     dest[dim_y] = pos[dim_y] + dir[dim_y];
//   }
// }

// void wanderer_class::copy(character_class *npc) {
//   this->pos[dim_x] = npc->pos[dim_x];
//   this->pos[dim_y] = npc->pos[dim_y];
//   this->symbol = npc->symbol;
//   this->next_turn = npc->next_turn;
//   this->ctype = npc->ctype;
//   this->mtype = (movement_type) move_wander;
//   this->dir[dim_x] = npc->dir[dim_x];
//   this->dir[dim_y] = npc->dir[dim_y];
//   this->defeated = true;
// }

// void sentry_class::move(pair_t& dest)
// {
//   // Not a bug.  Sentries are non-aggro.
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
// }

// void explorer_class::move(pair_t& dest)
// {
//   dest[dim_x] = pos[dim_x];
//   dest[dim_y] = pos[dim_y];
//   if (!defeated &&
//       world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                          [pos[dim_x] + dir[dim_x]] ==
//       &world.pc) {
//       io_battle(this, &world.pc);
//       return;
//   }
//   if ((move_cost[char_other][world.cur_map->map[pos[dim_y] +
//                                                 dir[dim_y]]
//                                                [pos[dim_x] +
//                                                 dir[dim_x]]] ==
//        INT_MAX) || world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                                       [pos[dim_x] + dir[dim_x]]) {
//     dir[dim_x] *= -1;
//     dir[dim_y] *= -1;
//   }
//   if ((move_cost[char_other][world.cur_map->map[pos[dim_y] +
//                                                 dir[dim_y]]
//                                                [pos[dim_x] +
//                                                 dir[dim_x]]] !=
//        INT_MAX) &&
//       !world.cur_map->cmap[pos[dim_y] + dir[dim_y]]
//                           [pos[dim_x] + dir[dim_x]]) {
//     dest[dim_x] = pos[dim_x] + dir[dim_x];
//     dest[dim_y] = pos[dim_y] + dir[dim_y];
//   }
// }

// void pc_class::move(pair_t& dest)
// {
//   io_display();
//   io_handle_input(dest);
// }

void move_character(character_class *c, pair_t dest)
{
  int min;
  int base;
  int i;
  switch (c->mtype)
  {
  case move_pc:
    io_display();
    io_handle_input(dest);
    break;
  case move_hiker:
    base = rand() & 0x7;

    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];
    min = INT_MAX;

    for (i = base; i < 8 + base; i++) {
      if ((world.hiker_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                           [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] <=
           min) &&
          !world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                              [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]]) {
        dest[dim_x] = c->pos[dim_x] + all_dirs[i & 0x7][dim_x];
        dest[dim_y] = c->pos[dim_y] + all_dirs[i & 0x7][dim_y];
        min = world.hiker_dist[dest[dim_y]][dest[dim_x]];
      }
      if (world.hiker_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                          [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] == 0) {
        io_battle(c, &world.pc);
        break;
      }
    }
    break;
  case move_rival:
    base = rand() & 0x7;

    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];
    min = INT_MAX;

    for (i = base; i < 8 + base; i++) {
      if ((world.rival_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                           [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] <
           min) &&
          !world.cur_map->cmap[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                              [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]]) {
        dest[dim_x] = c->pos[dim_x] + all_dirs[i & 0x7][dim_x];
        dest[dim_y] = c->pos[dim_y] + all_dirs[i & 0x7][dim_y];
        min = world.rival_dist[dest[dim_y]][dest[dim_x]];
      }
      if (world.rival_dist[c->pos[dim_y] + all_dirs[i & 0x7][dim_y]]
                          [c->pos[dim_x] + all_dirs[i & 0x7][dim_x]] == 0) {
        io_battle(c, &world.pc);
        break;
      }
    }
    break;
  case move_pace:
    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];

    if (!c->defeated &&
        world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ==
        &world.pc) {
        io_battle(c, &world.pc);
        return;
    }

    if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] !=
         world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) ||
        world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]]) {
      c->dir[dim_x] *= -1;
      c->dir[dim_y] *= -1;
    }

    if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ==
         world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) &&
        !world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                            [c->pos[dim_x] + c->dir[dim_x]]) {
      dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
      dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
    }
    break;
  case move_wander:
    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];

    if (!c->defeated &&
        world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ==
        &world.pc) {
        io_battle(c, &world.pc);
        return;
    }

    if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] !=
         world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) ||
        world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]]) {
      rand_dir(c->dir);
    }

    if ((world.cur_map->map[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ==
         world.cur_map->map[c->pos[dim_y]][c->pos[dim_x]]) &&
        !world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                            [c->pos[dim_x] + c->dir[dim_x]]) {
      dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
      dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
    }
    break;
  case move_walk:
    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];

    if (!c->defeated &&
        world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                           [c->pos[dim_x] + c->dir[dim_x]] ==
        &world.pc) {
        io_battle(c, &world.pc);
        return;
    }

    if ((move_cost[char_other][world.cur_map->map[c->pos[dim_y] +
                                                  c->dir[dim_y]]
                                                 [c->pos[dim_x] +
                                                  c->dir[dim_x]]] ==
         INT_MAX) || world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                                        [c->pos[dim_x] + c->dir[dim_x]]) {
      c->dir[dim_x] *= -1;
      c->dir[dim_y] *= -1;
    }

    if ((move_cost[char_other][world.cur_map->map[c->pos[dim_y] +
                                                  c->dir[dim_y]]
                                                 [c->pos[dim_x] +
                                                  c->dir[dim_x]]] !=
         INT_MAX) &&
        !world.cur_map->cmap[c->pos[dim_y] + c->dir[dim_y]]
                            [c->pos[dim_x] + c->dir[dim_x]]) {
      dest[dim_x] = c->pos[dim_x] + c->dir[dim_x];
      dest[dim_y] = c->pos[dim_y] + c->dir[dim_y];
    }
    break;
  case move_sentry:
    // Not a bug.  Sentries are non-aggro.
    dest[dim_x] = c->pos[dim_x];
    dest[dim_y] = c->pos[dim_y];
    break;
  default:
    break;
  }
}

int32_t cmp_char_turns(const void *key, const void *with)
{
  return ((character_class *) key)->next_turn - ((character_class *) with)->next_turn;
}

void delete_character(void *v)
{
  if (v == &world.pc) {
    // free((pc_class *) &world.pc);
  } else {
    free(((character_class *) v));
    free(v);
  }
}

#define ter_cost(x, y, c) move_cost[c][m->map[y][x]]

static int32_t hiker_cmp(const void *key, const void *with) {
  return (world.hiker_dist[((path *) key)->pos[dim_y]]
                          [((path *) key)->pos[dim_x]] -
          world.hiker_dist[((path *) with)->pos[dim_y]]
                          [((path *) with)->pos[dim_x]]);
}

static int32_t rival_cmp(const void *key, const void *with) {
  return (world.rival_dist[((path *) key)->pos[dim_y]]
                          [((path *) key)->pos[dim_x]] -
          world.rival_dist[((path *) with)->pos[dim_y]]
                          [((path *) with)->pos[dim_x]]);
}

void pathfind(map_class *m)
{
  heap_t h;
  uint32_t x, y;
  static path p[MAP_Y][MAP_X], *c;
  static uint32_t initialized = 0;

  if (!initialized) {
    initialized = 1;
    for (y = 0; y < MAP_Y; y++) {
      for (x = 0; x < MAP_X; x++) {
        p[y][x].pos[dim_y] = y;
        p[y][x].pos[dim_x] = x;
      }
    }
  }

  for (y = 0; y < MAP_Y; y++) {
    for (x = 0; x < MAP_X; x++) {
      world.hiker_dist[y][x] = world.rival_dist[y][x] = INT_MAX;
    }
  }
  world.hiker_dist[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = 
    world.rival_dist[world.pc.pos[dim_y]][world.pc.pos[dim_x]] = 0;

  heap_init(&h, hiker_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (ter_cost(x, y, char_hiker) != INT_MAX) {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      } else {
        p[y][x].hn = NULL;
      }
    }
  }

  while ((c = (path *) heap_remove_min(&h))) {
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x]    ] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x]    ] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y]    ][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y]    ][c->pos[dim_x] - 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y]    ][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y]    ][c->pos[dim_x] + 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x]    ] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x]    ] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker))) {
      world.hiker_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
        world.hiker_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_hiker);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);

  heap_init(&h, rival_cmp, NULL);

  for (y = 1; y < MAP_Y - 1; y++) {
    for (x = 1; x < MAP_X - 1; x++) {
      if (ter_cost(x, y, char_rival) != INT_MAX) {
        p[y][x].hn = heap_insert(&h, &p[y][x]);
      } else {
        p[y][x].hn = NULL;
      }
    }
  }

  while ((c = (path *) heap_remove_min(&h))) {
    c->hn = NULL;
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] - 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x]    ] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x]    ] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] - 1][c->pos[dim_x] + 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] - 1][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y]    ][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y]    ][c->pos[dim_x] - 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y]    ][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y]    ][c->pos[dim_x] + 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y]    ][c->pos[dim_x] + 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] - 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] - 1].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x]    ] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x]    ] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x]    ].hn);
    }
    if ((p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn) &&
        (world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] >
         world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
         ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival))) {
      world.rival_dist[c->pos[dim_y] + 1][c->pos[dim_x] + 1] =
        world.rival_dist[c->pos[dim_y]][c->pos[dim_x]] +
        ter_cost(c->pos[dim_x], c->pos[dim_y], char_rival);
      heap_decrease_key_no_replace(&h,
                                   p[c->pos[dim_y] + 1][c->pos[dim_x] + 1].hn);
    }
  }
  heap_delete(&h);
}
