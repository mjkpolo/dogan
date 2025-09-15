#pragma once

enum tile_type {
  BRICK,
  WOOD,
  SHEEP,
  WHEAT,
  STONE,
  DESERT,
};

enum toolbar_mode {
  TOOLBAR_NONE,
  TOOLBAR_START,
  TOOLBAR_MOVE,
};

enum BALANCE_LEVEL {
    None,    // m = 1024
    Low,     // m = 128
    Medium,  // m = 32
    High,    // m = 8
    Extreme  // m = 1
};

enum RoadType {
  ROAD_N_S,
  ROAD_NE_SW,
  ROAD_NW_SE,
};

enum PlayerType {
  PLAYER_ORANGE,
  PLAYER_RED,
  PLAYER_BLUE,
  PLAYER_WHITE,
};

