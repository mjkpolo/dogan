#include "dogan.hh"
#include "notcurses/notcurses.h"
#include "sprites/boat.hh"
#include "sprites/politics.hh"
#include "sprites/red_dice_1.hh"
#include "sprites/science.hh"
#include "sprites/trade.hh"
#include <algorithm>
#include <iostream>
#include <random>
#include <unistd.h>

std::vector<int> Dogan::bag_red;
std::vector<int> Dogan::bag_yellow;
std::vector<int> Dogan::bag_special;

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
  unsigned int offset = 0;
  offset += toolbar_->putstr(0, 0, " Ctrl + ") + 1;
  toolbar_->set_fg_rgb(base_rgb);
  toolbar_->set_bg_rgb(unselected_rgb);

  if (mode_ == TOOLBAR_START) {
    toolbar_->set_bg_rgb(selected_rgb);
  }
  offset += toolbar_->putstr(0, offset, " <s> START ") + 1;
  toolbar_->set_bg_rgb(unselected_rgb);

  if (mode_ == TOOLBAR_MOVE) {
    toolbar_->set_bg_rgb(selected_rgb);
  }
  offset += toolbar_->putstr(0, offset, " <m> MOVE ") + 1;
  toolbar_->set_bg_rgb(unselected_rgb);

  if (mode_ == TOOLBAR_MOVE) {
    offset += toolbar_->putstr(0, offset, " r ROLL ") + 1;
  }
}

static std::unique_ptr<ncpp::Plane> draw_popup(ncpp::Plane *n, int rows,
                                               int cols, int yoff, int xoff,
                                               unsigned rgb) {
  auto popup = std::make_unique<ncpp::Plane>(n, rows, cols, yoff, xoff);
  uint64_t channels = 0;
  ncchannels_set_fg_rgb(&channels, rgb);
  ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
  popup->double_box(0, channels, 5 - 1, 32 - 1, NCBOXMASK_TOP);
  ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
  popup->set_base("", 0, channels);

  return popup;
}

void Dogan::DrawBoard() {
  constexpr int x_req = water_border_w + 22;
  constexpr int y_req = water_border_h + 10;
  board_ = std::make_unique<ncpp::Plane>(y_, x_, 0, 0);
  uint64_t channels = 0;

  // TODO figure out what is actually needed
  unsigned int base_rgb = 0x232136;
  ncchannels_set_bg_rgb(&channels, base_rgb);
  ncchannels_set_bg_alpha(&channels, NCALPHA_OPAQUE);
  board_->set_base("", 0, channels);
  board_->set_fg_alpha(NCALPHA_TRANSPARENT);
  board_->set_bg_alpha(NCALPHA_OPAQUE);
  board_->set_bg_rgb(base_rgb);

  if (x_ < x_req || y_ * 2 < y_req) {
    auto popup = draw_popup(board_.get(), 5, 32, (int)y_ / 2,
                            (int)(x_ - 33) / 2, 0xf6c177);
    popup->putstr(0, 1, "TERMINAL TOO SMALL");
    popup->printf(1, 1, "NEED %d MORE ROWS, %d MORE COLS",
                  std::max(y_req - (int)y_ * 2, 0),
                  std::max(x_req - (int)x_, 0));

    for (int i = 3; i > 0; i--) {
      popup->printf(3, 1, "CLOSING IN %d...", i);
      nc_.render();
      sleep(1);
    }
    gameover_ = true;
    return;
  }

  DrawToolbar();

  DrawWaterBorder((y_ - water_border_h / 2) / 2, (x_ - water_border_w) / 2,
                  water_border_sprite);

  channels = 0;
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

  std::shuffle(random_tiles.begin(), random_tiles.end(), g_);
  std::shuffle(random_numbers.begin(), random_numbers.end(), g_);

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

  // unsigned y, x;
  // for (const auto &[pos, rt] : road_positions_) {
  //   y = pos.first;
  //   x = pos.second;
  //   DrawRoad(y, x, rt, PLAYER_RED);
  // }

  // for (const auto &[y, x] : building_positions_) {
  //   // DrawSettlement(y, x, PLAYER_RED);
  //   DrawCity(y, x, PLAYER_RED);
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

int draw_from_bag(std::vector<int> &bag, balance_level bl, std::mt19937 &gen,
                  bool takeoff) {
  unsigned int m;
  switch (bl) {
  case balance_level::None:
    m = 1024;
    break;
  case balance_level::Low:
    m = 128;
    break;
  case balance_level::Medium:
    m = 32;
    break;
  case balance_level::High:
    m = 8;
    break;
  case balance_level::Extreme:
    m = 1;
    break;
  }

  if (bag.empty()) {
    // std::cout << "BAG IS EMPTY" << std::endl;
    for (int face = 1; face <= 6; ++face) {
      for (unsigned int i = 0; i < m; ++i) {
        bag.push_back(face);
      }
    }
  }

  std::uniform_int_distribution<size_t> dist(0, bag.size() - 1);
  size_t idx = dist(gen);
  int val = bag[idx];
  if (takeoff) {
    bag.erase(bag.begin() + idx);
  }
  return val;
}

void Dogan::DrawDice(bool takeoff) {
  std::uniform_int_distribution<unsigned int> dist(1, 6);

  // unsigned int roll0 = dist(g_);
  // unsigned int roll1 = dist(g_);
  // unsigned int roll2 = dist(g_);
  unsigned int roll0 =
      draw_from_bag(bag_red, balance_level::Extreme, g_, takeoff);
  unsigned int roll1 =
      draw_from_bag(bag_yellow, balance_level::Extreme, g_, takeoff);
  unsigned int roll2 =
      draw_from_bag(bag_special, balance_level::Extreme, g_, takeoff);

  assert(roll0 >= 1 && roll0 <= 6);
  assert(roll1 >= 1 && roll1 <= 6);
  assert(roll2 >= 1 && roll2 <= 6);

  const uint32_t *sprite;
  switch (roll0) {
  case 1:
    sprite = red_dice_1_sprite;
    break;
  case 2:
    sprite = red_dice_2_sprite;
    break;
  case 3:
    sprite = red_dice_3_sprite;
    break;
  case 4:
    sprite = red_dice_4_sprite;
    break;
  case 5:
    sprite = red_dice_5_sprite;
    break;
  case 6:
    sprite = red_dice_6_sprite;
    break;
  }

  {
    std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)sprite, dice_length, dice_length * 4, dice_length);
    ncvisual_options vopts = {};
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
    vopts.n = board_->to_ncplane();
    vopts.y = y_ / 2 - dice_length / 4;
    vopts.x = x_ / 2 - dice_length * 3 / 2;
    dice0_ = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  }

  switch (roll1) {
  case 1:
    sprite = yellow_dice_1_sprite;
    break;
  case 2:
    sprite = yellow_dice_2_sprite;
    break;
  case 3:
    sprite = yellow_dice_3_sprite;
    break;
  case 4:
    sprite = yellow_dice_4_sprite;
    break;
  case 5:
    sprite = yellow_dice_5_sprite;
    break;
  case 6:
    sprite = yellow_dice_6_sprite;
    break;
  }

  {
    std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)sprite, dice_length, dice_length * 4, dice_length);

    ncvisual_options vopts = {};
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
    vopts.n = board_->to_ncplane();
    vopts.y = y_ / 2 - dice_length / 4;
    vopts.x = x_ / 2 - dice_length / 2;
    dice1_ = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  }

  switch (roll2) {
  case 1:
    sprite = boat_sprite;
    break;
  case 2:
    sprite = boat_sprite;
    break;
  case 3:
    sprite = boat_sprite;
    break;
  case 4:
    sprite = science_sprite;
    break;
  case 5:
    sprite = trade_sprite;
    break;
  case 6:
    sprite = politics_sprite;
    break;
  }

  {
    std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)sprite, dice_length, dice_length * 4, dice_length);

    ncvisual_options vopts = {};
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
    vopts.n = board_->to_ncplane();
    vopts.y = y_ / 2 - dice_length / 4;
    vopts.x = x_ / 2 + dice_length / 2;
    dice2_ = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
  }
}
