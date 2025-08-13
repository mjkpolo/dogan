#define NCPP_EXCEPTIONS_PLEASE
#include <Sprite.hh>
#include <assert.h>
#include <atomic>
#include <brick.hh>
#include <stone.hh>
#include <cassert>
#include <locale.h>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <stdio.h>
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
    static constexpr auto BOARD_WIDTH = 60;
    static constexpr auto BOARD_HEIGHT = 40;
    unsigned y, x;
    stdplane_->get_dim(&y, &x);
    board_top_y_ = y - (BOARD_HEIGHT + 2);
    board_left_x_ = x / 2 - (BOARD_WIDTH + 1);
    board_ = std::make_unique<ncpp::Plane>(BOARD_HEIGHT, BOARD_WIDTH * 2,
                                           board_top_y_, board_left_x_);
    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00b040);
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->double_box(0, channels, BOARD_HEIGHT - 1, BOARD_WIDTH * 2 - 1,
                       NCBOXMASK_TOP);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->set_base("", 0, channels);
    board_->printf(0, BOARD_WIDTH - strlen("DOGAN") / 2, "DOGAN");

    static constexpr int tile_side = 16;

    std::unique_ptr<ncpp::Visual> wood_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)wood_sprite, tile_side, tile_side * 4, tile_side);
    std::unique_ptr<ncpp::Visual> brick_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)brick_sprite, tile_side, tile_side * 4, tile_side);
    std::unique_ptr<ncpp::Visual> wheat_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)wheat_sprite, tile_side, tile_side * 4, tile_side);
    std::unique_ptr<ncpp::Visual> stone_ncv = std::make_unique<ncpp::Visual>(
        (const uint32_t *)stone_sprite, tile_side, tile_side * 4, tile_side);

    ncvisual_options vopts = {};
    vopts.leny = tile_side;
    vopts.lenx = tile_side;
    vopts.blitter = NCBLIT_2x1;
    vopts.flags = NCVISUAL_OPTION_NODEGRADE | NCVISUAL_OPTION_CHILDPLANE;
    vopts.n = board_->to_ncplane();

    constexpr int gap = tile_side+2;
    int start_y = gap/2;
    int start_x = gap/2;
    vopts.y = start_y;
    vopts.x = start_x;
    wood_ncv->blit(&vopts);
    vopts.y = start_y + 6;
    vopts.x = start_x + gap/2;
    brick_ncv->blit(&vopts);
    vopts.y = start_y - 6;
    vopts.x = start_x + gap/2;
    wheat_ncv->blit(&vopts);
    vopts.y = start_y;
    vopts.x = start_x + gap;
    stone_ncv->blit(&vopts);

    nc_.render();
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

  void DrawLegend() {
    static constexpr auto LEGEND_WIDTH = 12;
    static constexpr auto LEGEND_HEIGHT = 8;
    legend_ = std::make_unique<ncpp::Plane>(
        LEGEND_HEIGHT, LEGEND_WIDTH, board_top_y_ + 2, board_left_x_ + 2);
    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00b040);
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    legend_->double_box(0, channels, LEGEND_HEIGHT - 1, LEGEND_WIDTH - 1, 0);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    legend_->set_bg_alpha(NCALPHA_TRANSPARENT);
    legend_->set_fg_alpha(NCALPHA_TRANSPARENT);
    int yoff = 0;
    legend_->putstr(yoff, 1, "Legend:");
    yoff += 1;
    for (const auto &[tile, rgb] : tile_rgb) {
      legend_->set_fg_alpha(NCALPHA_OPAQUE);
      legend_->set_fg_rgb(rgb);
      int xoff = 1;
      legend_->putstr(yoff, xoff, "██");
      xoff += 4;
      legend_->set_fg_alpha(NCALPHA_TRANSPARENT);
      legend_->putstr(yoff, xoff, tile_name.at(tile));
      yoff += 1;
    }
    legend_->set_bg_alpha(NCALPHA_TRANSPARENT);
    legend_->set_base("", 0, channels);
  }

  void DrawTile(unsigned int rgb, int x, int y, const char *name, int num) {
    const size_t cols = 7;
    const size_t rows = 4;

    typedef enum {
      MID,
      TOP,
      BOT,
      U_L,
      U_R,
      L_L,
      L_R,
      EMP,
    } tile_texture;

    const tile_texture texture[rows][cols] = {
        {EMP, U_L, TOP, TOP, TOP, U_R, EMP},
        {U_L, MID, MID, MID, MID, MID, U_R},
        {L_L, MID, MID, MID, MID, MID, L_R},
        {EMP, L_L, BOT, BOT, BOT, L_R, EMP},
    };

    auto tile = std::make_unique<ncpp::Plane>(rows, cols, y, x, nullptr);
    uint64_t channels = 0;
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    tile->set_fg_rgb(rgb);
    tile->set_bg_alpha(NCALPHA_TRANSPARENT);
    tile->set_base("", 0, channels);

    char number[3];
    snprintf(number, 3, "%d", num);

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        if (num != -1 && i == rows / 2 - 1 && j >= cols / 2 &&
            j < cols / 2 + strlen(number)) {
          tile->putc(i, j, number[j - cols / 2]);
        } else {
          switch (texture[i][j]) {
          case MID:
            tile->putstr(i, j, "█");
            break;
          case U_L:
            tile->putstr(i, j, "🬵");
            break;
          case U_R:
            tile->putstr(i, j, "🬱");
            break;
          case L_L:
            tile->putstr(i, j, "🬊");
            break;
          case L_R:
            tile->putstr(i, j, "🬆");
            break;
          case TOP:
            tile->putstr(i, j, "🬹");
            break;
          case BOT:
            tile->putstr(i, j, "🬎");
            break;
          default:
            break;
          }
        }
      }
    }
    tiles_.push_back(std::move(tile));
  }

private:
  ncpp::NotCurses &nc_;
  uint64_t score_;
  std::mutex mtx_; // guards msdelay_
  std::unique_ptr<ncpp::Plane> board_;
  std::unique_ptr<ncpp::Plane> legend_;
  std::vector<std::pair<int, int>> positions_;
  std::vector<std::unique_ptr<ncpp::Plane>> tiles_;
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
  ncopts.loglevel = NCLOGLEVEL_WARNING;
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
