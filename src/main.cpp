#include "notcurses/notcurses.h"
#include <sys/_pthread/_pthread_types.h>
#define NCPP_EXCEPTIONS_PLEASE
#include <Sprite.hh>
#include <assert.h>
#include <atomic>
#include <brick.hh>
#include <cassert>
#include <desert.hh>
#include <locale.h>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <random>
#include <sheep.hh>
#include <stdio.h>
#include <stone.hh>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <wheat.hh>
#include <wood.hh>

static std::mutex ncmtx;

typedef enum { BRICK, WOOD, SHEEP, WHEAT, STONE, DESERT } tile_type;

const std::unordered_map<tile_type, int> tile_count = {
    {BRICK, 3}, {WOOD, 4}, {SHEEP, 4}, {WHEAT, 4}, {STONE, 3}, {DESERT, 1},
};

const std::unordered_map<int, int> number_count = {
    {12, 1}, {11, 2}, {10, 2}, {9, 2}, {8, 2},
    {6, 2},  {5, 2},  {4, 2},  {3, 2}, {2, 1},
};

const std::unordered_map<tile_type, const char *> tile_name = {
    {BRICK, "Brick"}, {WOOD, "Wood"},   {SHEEP, "Sheep"},
    {WHEAT, "Wheat"}, {STONE, "Stone"}, {DESERT, "Desert"},
};

const std::unordered_map<tile_type, const uint32_t *> tile_sprites = {
    {BRICK, brick_sprite}, {WOOD, wood_sprite},   {SHEEP, sheep_sprite},
    {WHEAT, wheat_sprite}, {STONE, stone_sprite}, {DESERT, desert_sprite},
};

const int total_tiles = tile_count.at(BRICK) + tile_count.at(WOOD) +
                        tile_count.at(SHEEP) + tile_count.at(WHEAT) +
                        tile_count.at(STONE) + tile_count.at(DESERT);

const std::unordered_map<tile_type, unsigned int> tile_rgb = {
    {BRICK, 0xfc9003}, {WOOD, 0x026317},  {SHEEP, 0x72f78f},
    {WHEAT, 0xe6f23d}, {STONE, 0x737373}, {DESERT, 0xcfcc95},
};

class Dogan {
public:
  Dogan(ncpp::NotCurses &nc, std::atomic_bool &gameover)
      : nc_(nc), stdplane_(nc_.get_stdplane()), gameover_(gameover),
        msdelay_(500) {
    DrawBoard();
  }

  static constexpr int tile_length = 16;

  void Ticker() { // FIXME ideally this would be called from constructor :/
    std::chrono::milliseconds ms;
    do {
      mtx_.lock();
      ms = msdelay_;
      mtx_.unlock();
      std::this_thread::sleep_for(ms);
      ncmtx.lock();
      ncmtx.unlock();
    } while (!gameover_);
  }

  void DrawBoard() {
    static constexpr int BOARD_WIDTH = 120;
    static constexpr int BOARD_HEIGHT = 40;

    unsigned y, x;
    stdplane_->get_dim(&y, &x);
    board_top_y_ = y - (BOARD_HEIGHT + 2);
    board_left_x_ = x / 2 - (BOARD_WIDTH / 2 + 1);
    board_ = std::make_unique<ncpp::Plane>(BOARD_HEIGHT, BOARD_WIDTH,
                                           board_top_y_, board_left_x_);
    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00b040);
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->double_box(0, channels, BOARD_HEIGHT - 1, BOARD_WIDTH - 1,
                       NCBOXMASK_TOP);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->set_base("", 0, channels);
    board_->printf(0, (BOARD_WIDTH - strlen("DOGAN")) / 2, "DOGAN");

    static constexpr int tile_center_y = (BOARD_HEIGHT - tile_length) / 2;
    static constexpr int tile_center_x = (BOARD_WIDTH - tile_length) / 2;
    assert(tile_length % 4 == 0);
    positions_ = {
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

    for (int i = 0; i < positions_.size(); i++) {
      tile_type tt = random_tiles[i];
      DrawTile(positions_[i].first, positions_[i].second, tile_sprites.at(tt));

      if (tt != DESERT) {
        int num = random_numbers.back();
        random_numbers.pop_back();
        auto number_tile = std::make_unique<ncpp::Plane>(
            board_.get(), 1, 3, positions_[i].first + tile_length / 4 - 1,
            positions_[i].second + tile_length / 2 - 2);
        uint64_t channels = 0;

        // TODO figure out what is actually needed
        ncchannels_set_bg_rgb(&channels, 0xf0d397);
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
        number_tile->set_bg_rgb(0xf0d397);
        number_tile->printf(0, 1, "%d", num);
        numbers_.push_back(std::move(number_tile));
      }
    }

    nc_.render();
  }

  void DrawTile(int y, int x, const uint32_t *sprite) {

    std::unique_ptr<ncpp::Visual> ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)sprite, tile_length, tile_length * 4, tile_length);

    ncvisual_options vopts = {};
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
    vopts.n = board_->to_ncplane();
    vopts.y = y;
    vopts.x = x;
    auto tile = std::make_unique<ncpp::Plane>(ncv->blit(&vopts));
    assert(tile.get() != nullptr);
    tiles_.push_back(std::move(tile));
  }

  void DrawSettlement(int y, int x) {
    static constexpr int SETTLE_WIDTH = 4;
    static constexpr int SETTLE_HEIGHT = 4;
    assert(SETTLE_WIDTH == blue_settlement.cols_);
    assert(SETTLE_HEIGHT == blue_settlement.rows_);
    std::unique_ptr<ncpp::Visual> blue_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)blue_settlement.pixels, SETTLE_HEIGHT,
        SETTLE_WIDTH * 4, SETTLE_WIDTH);

    std::unique_ptr<ncpp::Visual> red_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)orange_settlement.pixels, SETTLE_HEIGHT,
        SETTLE_WIDTH * 4, SETTLE_WIDTH);

    ncvisual_options vopts = {};
    vopts.leny = SETTLE_HEIGHT;
    vopts.lenx = SETTLE_WIDTH;
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE;
    auto settle = std::make_unique<ncpp::Plane>(2, SETTLE_WIDTH, y, x);
    vopts.n = settle->to_ncplane();

    // vopts.pxoffy = 1;
    red_ncv->blit(&vopts);
    vopts.flags |= NCVISUAL_OPTION_CHILDPLANE;
    blue_ncv->blit(&vopts);

    // settle->set_bg_alpha(NCALPHA_TRANSPARENT);
    // uint64_t channels = 0;
    // ncchannels_set_bg_alpha(&channels, NCALPHA_BLEND);
    // legend_->set_base("", 0, channels);
    settles_.push_back(std::move(settle));
  }

private:
  ncpp::NotCurses &nc_;
  uint64_t score_;
  std::mutex mtx_; // guards msdelay_
  std::unique_ptr<ncpp::Plane> board_;
  std::unique_ptr<ncpp::Plane> legend_;
  std::vector<std::pair<int, int>> positions_;
  std::vector<std::unique_ptr<ncpp::Plane>> tiles_;
  std::vector<std::unique_ptr<ncpp::Plane>> numbers_;
  std::vector<std::unique_ptr<ncpp::Plane>> settles_;
  ncpp::Plane *stdplane_;
  std::atomic_bool &gameover_;
  std::chrono::milliseconds msdelay_;
  unsigned int board_top_y_;
  unsigned int board_left_x_;
};

bool IOLoop(ncpp::NotCurses &nc, Dogan &t, std::atomic_bool &gameover) {
  ncpp::Plane *stdplane = nc.get_stdplane();
  char32_t input = 0;
  ncinput ni;
  while (!gameover && (input = nc.get(true, &ni)) != (char32_t)-1) {
    if (input == 'q') {
      break;
    }
    if (ni.evtype == ncpp::EvType::Release) {
      continue;
    }
    ncmtx.lock();
    switch (input) {
    default:
      stdplane->cursor_move(0, 0);
      stdplane->printf("Got unknown input U+%06x", input);
      nc.render();
      break;
    }
    ncmtx.unlock();
  }
  return gameover || input == 'q';
}

int main() {
  if (setlocale(LC_ALL, "") == nullptr) {
    return EXIT_FAILURE;
  }
  srand(time(nullptr));
  std::atomic_bool gameover = false;
  notcurses_options ncopts{};
  ncopts.flags = NCOPTION_INHIBIT_SETLOCALE;
  ncopts.loglevel = NCLOGLEVEL_DEBUG;
  ncpp::NotCurses nc(ncopts);
  {
    Dogan d{nc, gameover};
    std::thread tid(&Dogan::Ticker, &d);
    if (IOLoop(nc, d, gameover)) {
      gameover = true; // FIXME signal thread
      tid.join();
    } else {
      return EXIT_FAILURE;
    }
  }
  return nc.stop() ? EXIT_SUCCESS : EXIT_FAILURE;
}
