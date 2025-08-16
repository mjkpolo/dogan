#define NCPP_EXCEPTIONS_PLEASE
#include <Sprite.hh>
#include <assert.h>
#include <atomic>
#include <brick.hh>
#include <cassert>
#include <condition_variable>
#include <desert.hh>
#include <locale.h>
#include <mutex>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <random>
#include <sheep.hh>
#include <stdio.h>
#include <stone.hh>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <water_border.hh>
#include <wheat.hh>
#include <wood.hh>

static std::mutex ncmtx;
static std::condition_variable cv;
static std::condition_variable cv_done;
volatile bool tick_ready = false;

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
        msdelay_(50), board_drawn(false), numbers_placed(false),
        numbers_offset_(10) {
    DrawBoard();
  }

  static constexpr int tile_length = 16;
  static constexpr int border_width = 104;
  static constexpr int border_height = 68;

  void Ticker() { // FIXME ideally this would be called from constructor :/
    std::chrono::milliseconds ms;
    {
      std::lock_guard<std::mutex> l(mtx_);
      ms = msdelay_;
    }

    do {
      std::this_thread::sleep_for(ms);
      {
        std::lock_guard<std::mutex> l(ncmtx);
        tick_ready = true;
      }
      cv.notify_all();

      {
        std::unique_lock<std::mutex> l(ncmtx);
        cv_done.wait(l, [] { return !tick_ready; });
      }
    } while (!gameover_);
  }

  void Renderer() {
    do {
      std::unique_lock<std::mutex> l(ncmtx);
      cv.wait(l, [] { return tick_ready; });
      if (board_drawn && numbers_offset_ > 0) {
        move_numbers_down();
      }
      nc_.render();
      tick_ready = false;
      cv_done.notify_one();
    } while (!gameover_);
  }

  void DrawBoard() {
    static constexpr int BOARD_WIDTH = 120;
    static constexpr int BOARD_HEIGHT = 40;

    unsigned y, x;
    stdplane_->get_dim(&y, &x);
    board_top_y_ = y - (BOARD_HEIGHT + 2);
    board_left_x_ = x / 2 - (BOARD_WIDTH / 2 + 1);
    assert(border_height % 2 == 0);
    board_ = std::make_unique<ncpp::Plane>(BOARD_HEIGHT, BOARD_WIDTH,
                                           board_top_y_, board_left_x_);
    DrawWaterBorder(2, 3, water_border_sprite);

    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00b040);
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->double_box(0, channels, BOARD_HEIGHT - 1, BOARD_WIDTH - 1,
                       NCBOXMASK_TOP);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->set_base("", 0, channels);
    board_->putstr(0, (BOARD_WIDTH - strlen("DOGAN")) / 2, "DOGAN");

    static constexpr int tile_center_y = (BOARD_HEIGHT - tile_length) / 2 + 3;
    static constexpr int tile_center_x = (BOARD_WIDTH - tile_length) / 2 - 5;
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
            board_.get(), 1, 3,
            positions_[i].first + numbers_offset_ + tile_length / 4 - 1,
            positions_[i].second + tile_length / 2 - 2);
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

    nc_.render();
    board_drawn = true;
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

  void DrawTrade(const char *label, int y, int x) {
    auto trade = std::make_unique<ncpp::Plane>(water_border_.get(), 1, 3, y, x);

    ncplane_set_styles(trade->to_ncplane(), NCSTYLE_BOLD);
    trade->set_bg_alpha(NCALPHA_TRANSPARENT);
    trade->set_fg_alpha(NCALPHA_OPAQUE);
    trade->set_fg_rgb(0x0);
    trade->putstr(0, 0, label);

    trades_.push_back(std::move(trade));
  }

  void DrawWaterBorder(int y, int x, const uint32_t *sprite) {
    std::unique_ptr<ncpp::Visual> ncv =
        std::make_unique<ncpp::Visual>((const uint32_t *)sprite, border_height,
                                       border_width * 4, border_width);

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

  void move_numbers_down() {
    for (auto &number : numbers_) {
      int y, x;
      number->get_yx(y, x);
      number->move(y - 1, x);
    }
    numbers_offset_--;
  }

private:
  ncpp::NotCurses &nc_;
  uint64_t score_;
  std::mutex mtx_; // guards msdelay_
  std::unique_ptr<ncpp::Plane> board_;
  std::unique_ptr<ncpp::Plane> legend_;
  std::vector<std::pair<int, int>> positions_;
  std::unique_ptr<ncpp::Plane> water_border_;
  std::vector<std::unique_ptr<ncpp::Plane>> tiles_;
  std::vector<std::unique_ptr<ncpp::Plane>> numbers_;
  std::vector<std::unique_ptr<ncpp::Plane>> trades_;
  unsigned int numbers_offset_;
  std::vector<std::unique_ptr<ncpp::Plane>> settles_;
  ncpp::Plane *stdplane_;
  std::atomic_bool &gameover_;
  std::chrono::milliseconds msdelay_;
  unsigned int board_top_y_;
  unsigned int board_left_x_;
  volatile bool board_drawn;
  volatile bool numbers_placed;
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
    {
      std::lock_guard<std::mutex> l(ncmtx);
      switch (input) {
      default:
        stdplane->cursor_move(0, 0);
        stdplane->printf("Got unknown input U+%06x", input);
        nc.render();
        break;
      }
    }
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
    std::thread t_tid(&Dogan::Ticker, &d);
    std::thread r_tid(&Dogan::Renderer, &d);
    if (IOLoop(nc, d, gameover)) {
      gameover = true; // FIXME signal thread
      t_tid.join();
      r_tid.join();
    } else {
      return EXIT_FAILURE;
    }
  }
  return nc.stop() ? EXIT_SUCCESS : EXIT_FAILURE;
}
