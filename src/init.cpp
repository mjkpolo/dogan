#include "dogan.hh"

void Dogan::InitPositions() {
  static constexpr int tile_center_y = (water_border_h - tile_length) / 4;
  static constexpr int tile_center_x = (water_border_w - tile_length) / 2;
  assert(tile_length % 4 == 0);
  // TODO make this into a function like road positions
  tile_positions_ = {
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
  std::vector<std::pair<std::pair<int, int>, RoadType>> base_roads;

  // TODO simplify this even more
  //// w and e
  // center
  for (int i = 0; i < 6; i++) {
    base_roads.push_back({{2, -33 + i * tile_length}, ROAD_N_S});
  }
  // top 1 and bottom 1
  for (int i = 0; i < 5; i++) {
    base_roads.push_back(
        {{2 + 5, -33 + tile_length / 2 + i * tile_length}, ROAD_N_S});
    base_roads.push_back(
        {{2 - 5, -33 + tile_length / 2 + i * tile_length}, ROAD_N_S});
  }
  // top 2 and bottom 2
  for (int i = 0; i < 4; i++) {
    base_roads.push_back({{2 + 10, -33 + (i + 1) * tile_length}, ROAD_N_S});
    base_roads.push_back({{2 - 10, -33 + (i + 1) * tile_length}, ROAD_N_S});
  }
  //// sw and se
  // center
  for (int i = 0; i < 5; i++) {
    base_roads.push_back({{5, -33 + i * tile_length}, ROAD_NE_SW});
    base_roads.push_back({{5, -25 + i * tile_length}, ROAD_NW_SE});
  }
  // top 1 and bottom 1
  for (int i = 0; i < 4; i++) {
    base_roads.push_back(
        {{5 + 5, -33 + tile_length / 2 + i * tile_length}, ROAD_NE_SW});
    base_roads.push_back(
        {{5 + 5, -25 + tile_length / 2 + i * tile_length}, ROAD_NW_SE});
    base_roads.push_back(
        {{5 - 5, -33 + tile_length / 2 + i * tile_length}, ROAD_NE_SW});
    base_roads.push_back(
        {{5 - 5, -25 + tile_length / 2 + i * tile_length}, ROAD_NW_SE});
  }
  // extras:
  base_roads.push_back(
      {{5 - 5, -33 + tile_length / 2 + 4 * tile_length}, ROAD_NE_SW});
  base_roads.push_back(
      {{5 - 5, -25 + tile_length / 2 - 1 * tile_length}, ROAD_NW_SE});
  // top 2 and bottom 2
  for (int i = 0; i < 3; i++) {
    base_roads.push_back({{5 + 10, -33 + (i + 1) * tile_length}, ROAD_NE_SW});
    base_roads.push_back({{5 + 10, -25 + (i + 1) * tile_length}, ROAD_NW_SE});
    base_roads.push_back({{5 - 10, -33 + (i + 1) * tile_length}, ROAD_NE_SW});
    base_roads.push_back({{5 - 10, -25 + (i + 1) * tile_length}, ROAD_NW_SE});
  }
  // extras:
  base_roads.push_back({{5 - 10, -33 + 4 * tile_length}, ROAD_NE_SW});
  base_roads.push_back({{5 - 10, -25 + 0 * tile_length}, ROAD_NW_SE});

  // top 3
  for (int i = 0; i < 3; i++) {
    base_roads.push_back({{5 - 15, -25 + (i + 1) * tile_length}, ROAD_NE_SW});
    base_roads.push_back({{5 - 15, -33 + (i + 1) * tile_length}, ROAD_NW_SE});
  }

  for (auto &r : base_roads) {
    road_positions_.push_back(
        {{tile_center_y + r.first.first, tile_center_x + r.first.second},
         r.second});
  }
}
