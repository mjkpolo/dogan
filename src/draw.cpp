#include "dogan.hh"
#include "notcurses/notcurses.h"
#include <algorithm>
#include <random>
#include <unistd.h>

void Dogan::DrawToolbar() {
  toolbar_ = std::make_unique<ncpp::Plane>(1, x_, y_ - 1, 0);
  uint64_t channels = 0;

  unsigned int base_rgb = 0x2a273f;
  unsigned int text_rgb = 0xe0def4;
  unsigned int selected_rgb = 0xeb6f92;
  unsigned int unselected_rgb = 0x6e6a86;
  ncchannels_set_bg_rgb(&channels, base_rgb);
  ncchannels_set_bg_alpha(&channels, NCALPHA_OPAQUE);
  toolbar_->set_base("", 0, channels);
  toolbar_->set_fg_alpha(NCALPHA_OPAQUE);
  toolbar_->set_bg_alpha(NCALPHA_OPAQUE);
  ncplane_set_styles(toolbar_->to_ncplane(), NCSTYLE_BOLD);

  toolbar_->set_fg_rgb(text_rgb);
  toolbar_->putstr(0, 0, " Ctrl + ");
  toolbar_->set_fg_rgb(base_rgb);

  if (mode_ == TOOLBAR_START)
    toolbar_->set_bg_rgb(selected_rgb);
  else
    toolbar_->set_bg_rgb(unselected_rgb);
  toolbar_->putstr(0, 8, " <s> START ");
  if (mode_ == TOOLBAR_MOVE)
    toolbar_->set_bg_rgb(selected_rgb);
  else
    toolbar_->set_bg_rgb(unselected_rgb);
  toolbar_->putstr(0, 20, " <m> MOVE ");
}

void Dogan::DrawBoard() {
  constexpr int x_req = water_border_w + 22;
  constexpr int y_req = water_border_h + 10;
  if (x_ < x_req || y_ * 2 < y_req) {
    board_ = std::make_unique<ncpp::Plane>(y_, x_, 0, 0);
    board_->putstr(y_ / 2, (x_ - 18) / 2, "TERMINAL TOO SMALL");
    board_->printf(y_ / 2 + 1, (x_ - 32) / 2, "NEED %d MORE ROWS, %d MORE COLS",
                   std::max(y_req - (int)y_ * 2, 0),
                   std::max(x_req - (int)x_, 0));

    for (int i = 3; i > 0; i--) {
      board_->printf(y_ / 2 + 3, (x_ - 16) / 2, "CLOSING IN %d...", i);
      nc_.render();
      sleep(1);
    }
    gameover_ = true;
    return;
  }

  board_ = std::make_unique<ncpp::Plane>(y_, x_, 0, 0);
  DrawToolbar();

  DrawWaterBorder((y_ - water_border_h / 2) / 2, (x_ - water_border_w) / 2,
                  water_border_sprite);

  uint64_t channels = 0;
  board_->putstr(0, (x_ - strlen("DOGAN")) / 2, "DOGAN");

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

  // for (const auto &[pos, rt] : road_positions_) {
  //   y = pos.first;
  //   x = pos.second;
  //   DrawRoad(y, x, rt, PLAYER_BLUE);
  // }

  // for (const auto &[y, x] : building_positions_) {
  //   DrawCity(y, x, PLAYER_BLUE);
  // }

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
  // WARN subtract 1 from x to match settlement
  x -= 1;
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
