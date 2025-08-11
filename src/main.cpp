#include <cstdlib>
#define NCPP_EXCEPTIONS_PLEASE
#include <algorithm>
#include <assert.h>
#include <locale.h>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <random>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

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
    {BRICK, "BR"}, {WOOD, "WO"},  {SHEEP, "SH"},
    {WHEAT, "WH"}, {STONE, "ST"}, {DESERT, "DE"},
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
    static constexpr auto BOARD_WIDTH = 40;
    static constexpr auto BOARD_HEIGHT = 30;
    unsigned y, x;
    stdplane_->get_dim(&y, &x);
    board_top_y_ = y - (BOARD_HEIGHT + 2);
    board_ = std::make_unique<ncpp::Plane>(
        BOARD_HEIGHT, BOARD_WIDTH * 2, board_top_y_, x / 2 - (BOARD_WIDTH + 1));
    uint64_t channels = 0;
    ncchannels_set_fg_rgb(&channels, 0x00b040);
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->double_box(0, channels, BOARD_HEIGHT - 1, BOARD_WIDTH * 2 - 1,
                       NCBOXMASK_TOP);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    board_->set_base("", 0, channels);
    board_->printf(0, BOARD_WIDTH - strlen("DOGAN") / 2, "DOGAN");
    nc_.render();
    int tmpx = x / 2 - 3;
    int tmpy = y / 2 - 2;

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

    const std::vector<std::pair<int, int>> positions = {
        {tmpx - 12, tmpy - 4}, {tmpx - 12, tmpy + 4}, {tmpx - 12, tmpy},
        {tmpx - 6, tmpy - 2},  {tmpx - 6, tmpy - 6},  {tmpx - 6, tmpy + 2},
        {tmpx - 6, tmpy + 6},  {tmpx + 12, tmpy - 4}, {tmpx + 12, tmpy + 4},
        {tmpx + 12, tmpy},     {tmpx + 6, tmpy - 2},  {tmpx + 6, tmpy - 6},
        {tmpx + 6, tmpy + 2},  {tmpx + 6, tmpy + 6},  {tmpx, tmpy - 4},
        {tmpx, tmpy - 8},      {tmpx, tmpy + 4},      {tmpx, tmpy + 8},
        {tmpx, tmpy},
    };

    int saw_desert = 0;
    for (int i = 0; i < positions.size(); i++) {
      DrawTile(tile_rgb.at(random_tiles[i]), positions[i].first,
               positions[i].second, tile_name.at(random_tiles[i]),
               random_numbers[i - saw_desert]);
      if (random_tiles[i] == DESERT) {
        saw_desert = 1;
      }
    }
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
        if (i == rows / 2 - 1 && j >= cols / 2 &&
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
    nc_.render();
    tiles_.push_back(std::move(tile));
  }

private:
  ncpp::NotCurses &nc_;
  uint64_t score_;
  std::mutex mtx_; // guards msdelay_
  std::unique_ptr<ncpp::Plane> board_;
  std::vector<std::unique_ptr<ncpp::Plane>> tiles_;
  ncpp::Plane *stdplane_;
  std::atomic_bool &gameover_;
  std::chrono::milliseconds msdelay_;
  int board_top_y_;
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
