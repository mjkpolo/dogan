#include "dogan.hh"

// WARN this is horrible...
// I tried to be clever,
// but it's basically hardcoded.

// TODO make this into a function like road positions
static std::vector<std::pair<int, int>>
init_tile_positions(const int tile_center_y, const int tile_center_x,
                    const int tile_length) {
  return {
      {tile_center_y, tile_center_x},
      {tile_center_y + tile_length / 4 + 1,
       tile_center_x + 1 * (tile_length / 2)},
      {tile_center_y - (tile_length / 4 + 1),
       tile_center_x + 1 * (tile_length / 2)},
      {tile_center_y + tile_length / 4 + 1,
       tile_center_x - 1 * (tile_length / 2)},
      {tile_center_y - (tile_length / 4 + 1),
       tile_center_x - 1 * (tile_length / 2)},

      {tile_center_y + 2 * (tile_length / 4 + 1),
       tile_center_x + 0 * (tile_length / 2)},
      {tile_center_y - 2 * (tile_length / 4 + 1),
       tile_center_x - 0 * (tile_length / 2)},
      {tile_center_y + 0 * (tile_length / 4 + 1),
       tile_center_x + 2 * (tile_length / 2)},
      {tile_center_y - 0 * (tile_length / 4 + 1),
       tile_center_x - 2 * (tile_length / 2)},

      {tile_center_y + 2 * (tile_length / 4 + 1),
       tile_center_x + 2 * (tile_length / 2)},
      {tile_center_y + 2 * (tile_length / 4 + 1),
       tile_center_x - 2 * (tile_length / 2)},
      {tile_center_y - 2 * (tile_length / 4 + 1),
       tile_center_x + 2 * (tile_length / 2)},
      {tile_center_y - 2 * (tile_length / 4 + 1),
       tile_center_x - 2 * (tile_length / 2)},

      {tile_center_y + 1 * (tile_length / 4 + 1),
       tile_center_x + 3 * (tile_length / 2)},
      {tile_center_y + 1 * (tile_length / 4 + 1),
       tile_center_x - 3 * (tile_length / 2)},
      {tile_center_y - 1 * (tile_length / 4 + 1),
       tile_center_x + 3 * (tile_length / 2)},
      {tile_center_y - 1 * (tile_length / 4 + 1),
       tile_center_x - 3 * (tile_length / 2)},

      {tile_center_y + 0 * (tile_length / 4 + 1),
       tile_center_x + 4 * (tile_length / 2)},
      {tile_center_y + 0 * (tile_length / 4 + 1),
       tile_center_x - 4 * (tile_length / 2)},
  };
}

static std::vector<std::pair<std::pair<int, int>, RoadType>>
init_road_positions(const int tile_center_y, const int tile_center_x,
                    const int tile_length) {
  std::vector<std::pair<std::pair<int, int>, RoadType>> road_positions;
  // TODO simplify this even more
  //// w and e
  // center
  for (int i = 0; i < 6; i++) {
    road_positions.push_back(
        {{tile_center_y + 2, tile_center_x - 33 + i * tile_length}, ROAD_N_S});
  }
  // top 1 and bottom 1
  for (int i = 0; i < 5; i++) {
    road_positions.push_back(
        {{tile_center_y + 2 + 5,
          tile_center_x - 33 + tile_length / 2 + i * tile_length},
         ROAD_N_S});
    road_positions.push_back(
        {{tile_center_y + 2 - 5,
          tile_center_x - 33 + tile_length / 2 + i * tile_length},
         ROAD_N_S});
  }
  // top 2 and bottom 2
  for (int i = 0; i < 4; i++) {
    road_positions.push_back(
        {{tile_center_y + 2 + 10, tile_center_x - 33 + (i + 1) * tile_length},
         ROAD_N_S});
    road_positions.push_back(
        {{tile_center_y + 2 - 10, tile_center_x - 33 + (i + 1) * tile_length},
         ROAD_N_S});
  }
  //// sw and se
  // center
  for (int i = 0; i < 5; i++) {
    road_positions.push_back(
        {{tile_center_y + 5, tile_center_x - 33 + i * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5, tile_center_x - 25 + i * tile_length},
         ROAD_NW_SE});
  }
  // top 1 and bottom 1
  for (int i = 0; i < 4; i++) {
    road_positions.push_back(
        {{tile_center_y + 5 + 5,
          tile_center_x - 33 + tile_length / 2 + i * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5 + 5,
          tile_center_x - 25 + tile_length / 2 + i * tile_length},
         ROAD_NW_SE});
    road_positions.push_back(
        {{tile_center_y + 5 - 5,
          tile_center_x - 33 + tile_length / 2 + i * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5 - 5,
          tile_center_x - 25 + tile_length / 2 + i * tile_length},
         ROAD_NW_SE});
  }
  // extras:
  road_positions.push_back(
      {{tile_center_y + 5 - 5,
        tile_center_x - 33 + tile_length / 2 + 4 * tile_length},
       ROAD_NE_SW});
  road_positions.push_back(
      {{tile_center_y + 5 - 5,
        tile_center_x - 25 + tile_length / 2 - 1 * tile_length},
       ROAD_NW_SE});
  // top 2 and bottom 2
  for (int i = 0; i < 3; i++) {
    road_positions.push_back(
        {{tile_center_y + 5 + 10, tile_center_x - 33 + (i + 1) * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5 + 10, tile_center_x - 25 + (i + 1) * tile_length},
         ROAD_NW_SE});
    road_positions.push_back(
        {{tile_center_y + 5 - 10, tile_center_x - 33 + (i + 1) * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5 - 10, tile_center_x - 25 + (i + 1) * tile_length},
         ROAD_NW_SE});
  }
  // extras:
  road_positions.push_back(
      {{tile_center_y + 5 - 10, tile_center_x - 33 + 4 * tile_length},
       ROAD_NE_SW});
  road_positions.push_back(
      {{tile_center_y + 5 - 10, tile_center_x - 25 + 0 * tile_length},
       ROAD_NW_SE});

  // top 3
  for (int i = 0; i < 3; i++) {
    road_positions.push_back(
        {{tile_center_y + 5 - 15, tile_center_x - 25 + (i + 1) * tile_length},
         ROAD_NE_SW});
    road_positions.push_back(
        {{tile_center_y + 5 - 15, tile_center_x - 33 + (i + 1) * tile_length},
         ROAD_NW_SE});
  }

  return road_positions;
}

static std::vector<std::pair<int, int>>
init_building_positions(const int tile_center_y, const int tile_center_x,
                        const int tile_length) {

  std::vector<std::pair<int, int>> building_positions;

  // nw, se, sw, se
  for (int i = 0; i < 6; i++) {
    building_positions.push_back(
        {tile_center_y + 1, tile_center_x - 35 + i * tile_length});
    building_positions.push_back(
        {tile_center_y + 4, tile_center_x - 35 + i * tile_length});
    if (i > 0) {
      building_positions.push_back(
          {tile_center_y + 3 - (tile_length / 4),
           tile_center_x - 35 - tile_length / 2 + i * tile_length});
      building_positions.push_back(
          {tile_center_y + 2 + (tile_length / 4),
           tile_center_x - 35 - tile_length / 2 + i * tile_length});
      building_positions.push_back(
          {tile_center_y + 1 + (tile_length / 2),
           tile_center_x - 35 - tile_length / 2 + i * tile_length});
      building_positions.push_back(
          {tile_center_y + 4 - (tile_length / 2),
           tile_center_x - 35 - tile_length / 2 + i * tile_length});
      if (i < 5) {
        building_positions.push_back({tile_center_y + 1 - (tile_length / 2 - 1),
                                      tile_center_x - 35 + i * tile_length});
        building_positions.push_back({tile_center_y - 2 - (tile_length / 2 - 1),
                                      tile_center_x - 35 + i * tile_length});
        building_positions.push_back({tile_center_y + 4 + (tile_length / 2 - 1),
                                      tile_center_x - 35 + i * tile_length});
        building_positions.push_back({tile_center_y + 7 + (tile_length / 2 - 1),
                                      tile_center_x - 35 + i * tile_length});
        if (i > 1) {
          building_positions.push_back(
              {tile_center_y + 0 + (tile_length),
               tile_center_x - 35 - tile_length / 2 + i * tile_length});
          building_positions.push_back(
              {tile_center_y + 5 - (tile_length),
               tile_center_x - 35 - tile_length / 2 + i * tile_length});
        }
      }
    }
  }
  // nw, se, sw, se
  return building_positions;
}

void Dogan::InitPositions() {
  static constexpr int tile_center_y = (water_border_h - tile_length) / 4;
  static constexpr int tile_center_x = (water_border_w - tile_length) / 2;
  assert(tile_length % 4 == 0);
  tile_positions_ =
      init_tile_positions(tile_center_y, tile_center_x, tile_length);
  road_positions_ =
      init_road_positions(tile_center_y, tile_center_x, tile_length);
  building_positions_ =
      init_building_positions(tile_center_y, tile_center_x, tile_length);
}
