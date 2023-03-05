#ifndef CHARACTER_HPP
# define CHARACTER_HPP

#include "stdio.h"
#include "iostream"

// #include "poke327.hpp"
#include "enums.hpp"

typedef int16_t pair_t[2];

extern const char *char_type_name[num_character_types];

extern int32_t move_cost[num_character_types][num_terrain_types];

// class character {
//   public:
//     character(character_type ctype, movement_type mtype, pair_t dir) {
//       this->ctype = ctype;
//       this->mtype = mtype;
//       this->dir = dir;
//     }

//     character_type ctype;
//     movement_type mtype;
//     pair_t dir;
// };

class map_class;
class character_class;

class character_class {
  // npc_t *npc;
  // pc_t *pc;
  public:
    character_class() {}
    character_class(character_type ctype, movement_type mtype, pair_t pos, pair_t dir, int next_turn, char symbol)
    {
      this->pos[dim_x] = pos[dim_x];
      this->pos[dim_y] = pos[dim_y];
      this->symbol = symbol;
      this->next_turn = next_turn;
      this->ctype = ctype;
      this->mtype = mtype;
      this->dir[dim_x] = dir[dim_x];
      this->dir[dim_y] = dir[dim_y];
      this->defeated = false;
    }

    // virtual void move(pair_t& dest);
    // virtual void copy(character_class *npc);
    character_type ctype;
    movement_type mtype;
    pair_t pos;
    pair_t dir;
    int next_turn;
    char symbol;
    bool defeated;
};

// class hiker_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

// class rival_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

// class pacer_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

// class wanderer_class : public character_class {
//   public:
//     // using character_class::copy;
//     virtual void copy(character_class *npc);
//     void move(pair_t& dest);
// };

// class sentry_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

// class explorer_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

// class pc_class : public character_class {
//   public:
//     void move(pair_t& dest);
// };

/* character is defined in poke327.h to allow an instance of character
 * in world without including character.h in poke327.h                 */

int32_t cmp_char_turns(const void *key, const void *with);
void delete_character(void *v);
void pathfind(map_class *m);
void move_character(character_class *c, pair_t dest);

int pc_move(char);

#endif
