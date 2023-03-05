#ifndef ENUMS_HPP
# define ENUMS_HPP

enum dim {
  dim_x,
  dim_y,
  num_dims
};

enum terrain_type {
  ter_boulder,
  ter_tree,
  ter_path,
  ter_mart,
  ter_center,
  ter_grass,
  ter_clearing,
  ter_mountain,
  ter_forest,
  ter_exit,
  num_terrain_types
};

enum movement_type {
  move_hiker,
  move_rival,
  move_pace,
  move_wander,
  move_sentry,
  move_walk,
  move_pc,
  num_movement_types
};

enum character_type {
  char_pc,
  char_hiker,
  char_rival,
  char_other,
  num_character_types
};

#endif