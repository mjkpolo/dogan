#include "dogan.hh"

void Dogan::DrawBoard() {
  unsigned y, x;
  stdplane_->get_dim(&y, &x);
  board_top_y_ = y - (board_h + 2);
  board_left_x_ = x / 2 - (board_w / 2 + 1);
  board_ = std::make_unique<ncpp::Plane>(board_h, board_w, board_top_y_,
                                         board_left_x_);

  DrawWaterBorder((board_h - water_border_h / 2) / 2,
                  (board_w - water_border_w) / 2, water_border_sprite);

  DrawSettlement(5, 5, PLAYER_WHITE);
  DrawCity(7, 5, PLAYER_WHITE);

  uint64_t channels = 0;
  ncchannels_set_fg_rgb(&channels, 0x00b040);
  ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
  board_->double_box(0, channels, board_h - 1, board_w - 1, NCBOXMASK_TOP);
  ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
  board_->set_base("", 0, channels);
  board_->putstr(0, (board_w - strlen("DOGAN")) / 2, "DOGAN");

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

  std::vector<tile_type> random_tiles;

  for (const auto &[tile, count] : tile_count) {
    for (int i = 0; i < count; ++i) {
      random_tiles.push_back(tile);
    }
  }

  std::vector<int> random_numbers;
  for (const auto &[num, count] : number_count) {
    for (int i = 0; i < count; ++i) {
      random_numbers.push_back(num);
    }
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(random_tiles.begin(), random_tiles.end(), g);
  std::shuffle(random_numbers.begin(), random_numbers.end(), g);

  for (int i = 0; i < tile_positions_.size(); i++) {
    tile_type tt = random_tiles[i];
    DrawTile(tile_positions_[i].first, tile_positions_[i].second,
             tile_sprites.at(tt));

    if (tt != DESERT) {
      int num = random_numbers.back();
      random_numbers.pop_back();
      auto number_tile = std::make_unique<ncpp::Plane>(
          water_border_.get(), 1, 3,
          tile_positions_[i].first + numbers_offset_ + tile_length / 4 - 1,
          tile_positions_[i].second + tile_length / 2 - 2);
      uint64_t channels = 0;

      // TODO figure out what is actually needed
      unsigned int base_rgb = 0xfdecbc;
      ncchannels_set_bg_rgb(&channels, base_rgb);
      ncchannels_set_bg_alpha(&channels, NCALPHA_OPAQUE);

      unsigned int stylebits = NCSTYLE_BOLD;
      if (num == 6 || num == 9) {
        stylebits |= NCSTYLE_UNDERLINE;
      }

      ncplane_set_styles(number_tile->to_ncplane(), stylebits);
      if (num == 6 || num == 8) {
        number_tile->set_fg_rgb(0xff2020);
      } else {
        number_tile->set_fg_rgb(0x0);
      }

      number_tile->set_base(" ", 0, channels);
      number_tile->set_fg_alpha(NCALPHA_OPAQUE);
      number_tile->set_bg_alpha(NCALPHA_OPAQUE);
      number_tile->set_bg_rgb(base_rgb);
      number_tile->printf(0, 1, "%d", num);
      numbers_.push_back(std::move(number_tile));
    }
  }

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

  for (const auto &[pos, rt] : road_positions_) {
    y = pos.first;
    x = pos.second;
    DrawRoad(y, x, rt, PLAYER_BLUE);
  }
  nc_.render();
  board_drawn = true;
}

void Dogan::DrawTile(int y, int x, const uint32_t *sprite) {
  std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
      (const uint32_t *)sprite, tile_length, tile_length * 4, tile_length);

  ncvisual_options vopts = {};
  vopts.blitter = NCBLIT_2x1;
  vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
  vopts.n = water_border_->to_ncplane();
  vopts.y = y;
  vopts.x = x;
  auto tile = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  assert(tile.get() != nullptr);
  tiles_.push_back(std::move(tile));
}

void Dogan::DrawTrade(const char *label, int y, int x) {
  auto trade = std::make_unique<ncpp::Plane>(water_border_.get(), 1, 3, y, x);

  ncplane_set_styles(trade->to_ncplane(), NCSTYLE_BOLD);
  trade->set_bg_alpha(NCALPHA_TRANSPARENT);
  trade->set_fg_alpha(NCALPHA_OPAQUE);
  trade->set_fg_rgb(0x0);
  trade->putstr(0, 0, label);

  trades_.push_back(std::move(trade));
}

void Dogan::DrawWaterBorder(int y, int x, const uint32_t *sprite) {
  std::unique_ptr<ncpp::Visual> ncv =
      std::make_unique<ncpp::Visual>((const uint32_t *)sprite, water_border_h,
                                     water_border_w * 4, water_border_w);

  ncvisual_options vopts = {};
  vopts.blitter = NCBLIT_2x1;
  vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
  vopts.n = board_->to_ncplane();
  vopts.y = y;
  vopts.x = x;
  water_border_ = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));

  DrawTrade("3:1", 1, 27);
  DrawTrade("2:1", 2, 58);
  DrawTrade("3:1", 6, 81);
  DrawTrade("3:1", 16, 96);
  DrawTrade("2:1", 27, 80);
  DrawTrade("2:1", 32, 58);
  DrawTrade("3:1", 31, 27);
  DrawTrade("2:1", 22, 11);
  DrawTrade("2:1", 12, 11);
}

void Dogan::DrawSettlement(int y, int x, PlayerType pt) {
  const uint32_t *sprite;
  switch (pt) {
  case PLAYER_BLUE:
    sprite = blue_settlement_sprite;
    break;
  case PLAYER_RED:
    sprite = red_settlement_sprite;
    break;
  case PLAYER_ORANGE:
    sprite = orange_settlement_sprite;
    break;
  case PLAYER_WHITE:
    sprite = white_settlement_sprite;
    break;
  }
  auto ncv = std::make_unique<ncpp::Visual>((const uint32_t *)sprite, settle_h,
                                            settle_w * 4, settle_w);

  ncvisual_options vopts = {};
  vopts.blitter = NCBLIT_2x1;
  vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
  vopts.y = y;
  vopts.x = x;
  vopts.n = water_border_->to_ncplane();
  auto settlement = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  settles_.push_back(std::move(settlement));
}

void Dogan::DrawCity(int y, int x, PlayerType pt) {
  const uint32_t *sprite;
  switch (pt) {
  case PLAYER_BLUE:
    sprite = blue_city_sprite;
    break;
  case PLAYER_RED:
    sprite = red_city_sprite;
    break;
  case PLAYER_ORANGE:
    sprite = orange_city_sprite;
    break;
  case PLAYER_WHITE:
    sprite = white_city_sprite;
    break;
  }

  std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
      (const uint32_t *)sprite, city_h, city_w * 4, city_w);

  ncvisual_options vopts = {};
  vopts.blitter = NCBLIT_2x1;
  vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
  vopts.n = water_border_->to_ncplane();
  vopts.y = y;
  vopts.x = x;
  auto city = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  cities_.push_back(std::move(city));
}

void Dogan::DrawRoad(int y, int x, RoadType rt, PlayerType pt) {
  int road_h;
  int road_w;
  const uint32_t *sprite;
  switch (rt) {
  case ROAD_NE_SW:
    road_h = 6;
    road_w = 8;
    switch (pt) {
    case PLAYER_BLUE:
      sprite = blue_road_ne_sw_sprite;
      break;
    case PLAYER_WHITE:
      sprite = white_road_ne_sw_sprite;
      break;
    case PLAYER_ORANGE:
      sprite = orange_road_ne_sw_sprite;
      break;
    case PLAYER_RED:
      sprite = red_road_ne_sw_sprite;
      break;
    }
    break;
  case ROAD_NW_SE:
    road_h = 6;
    road_w = 8;
    switch (pt) {
    case PLAYER_BLUE:
      sprite = blue_road_nw_se_sprite;
      break;
    case PLAYER_WHITE:
      sprite = white_road_nw_se_sprite;
      break;
    case PLAYER_ORANGE:
      sprite = orange_road_nw_se_sprite;
      break;
    case PLAYER_RED:
      sprite = red_road_nw_se_sprite;
      break;
    }
    break;
  case ROAD_N_S:
    road_h = 6;
    road_w = 2;
    switch (pt) {
    case PLAYER_BLUE:
      sprite = blue_road_n_s_sprite;
      break;
    case PLAYER_WHITE:
      sprite = white_road_n_s_sprite;
      break;
    case PLAYER_ORANGE:
      sprite = orange_road_n_s_sprite;
      break;
    case PLAYER_RED:
      sprite = red_road_n_s_sprite;
      break;
    }
    break;
  }

  auto ncv = std::make_unique<ncpp::Visual>((const uint32_t *)sprite, road_h,
                                            road_w * 4, road_w);
  ncvisual_options vopts = {};
  vopts.blitter = NCBLIT_2x1;
  vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
  vopts.n = water_border_->to_ncplane();
  vopts.y = y;
  vopts.x = x;
  auto road = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  roads_.push_back(std::move(road));
}
