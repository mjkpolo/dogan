#ifndef DOGAN_DOGAN
#define DOGAN_DOGAN

#include "sprites.hh"
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <random>
#include <thread>
#include <unordered_map>
#include <atomic>

extern std::mutex ncmtx;
static std::condition_variable cv;
static std::condition_variable cv_done;
static volatile bool tick_ready = false;

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

struct Player {
  unsigned int id;
  std::string name;
  PlayerType type;
};


class Dogan {
public:
  Dogan(ncpp::NotCurses &nc, std::atomic_bool &gameover)
      : nc_(nc), stdplane_(nc_.get_stdplane()), gameover_(gameover),
        msdelay_(100), board_drawn(false), numbers_offset_(10),
        mode_(TOOLBAR_NONE), g_(rd_()) {
    stdplane_->get_dim(&y_, &x_);
    InitPositions();
    DrawBoard();
  }

  void load_config(const std::string& filename);
  void set_mode(toolbar_mode mode) { mode_ = mode; }
  void cleanup_mode() {
    switch (mode_) {
    case TOOLBAR_MOVE:
      dice0_ = nullptr;
      dice1_ = nullptr;
      dice2_ = nullptr;
      break;
    case TOOLBAR_START:
      break;
    case TOOLBAR_NONE:
      break;
    }
  }
  toolbar_mode get_mode() { return mode_; }

  static constexpr int water_border_w = 104;
  static constexpr int water_border_h = 68;
  static constexpr int tile_length = 16;
  static constexpr int dice_length = 36;
  static constexpr int settle_w = 6;
  static constexpr int settle_h = 4;
  static constexpr int city_w = 8;
  static constexpr int city_h = 4;
  static constexpr int player_num = 0;
  static std::vector<Player> players;
  BALANCE_LEVEL bl = BALANCE_LEVEL::Extreme;

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

  void DrawToolbar();

  void DrawDice(bool takeoff=false);
  // int draw_from_bag(std::vector<int> &bag, unsigned int m, std::mt19937 &gen, bool takeoff);


private:
  void InitPositions();

  void DrawBoard();

  void DrawTile(int y, int x, const uint32_t *sprite);

  void DrawTrade(const char *label, int y, int x);

  void DrawWaterBorder(int y, int x, const uint32_t *sprite);

  void DrawSettlement(int y, int x, PlayerType pt);

  void DrawCity(int y, int x, PlayerType pt);

  void DrawRoad(int y, int x, RoadType rt, PlayerType pt);

  void move_numbers_down() {
    for (auto &number : numbers_) {
      int y, x;
      number->get_yx(y, x);
      number->move(y - 1, x);
    }
    numbers_offset_--;
  }

  ncpp::NotCurses &nc_;
  std::mutex mtx_; // guards msdelay_

  toolbar_mode mode_;

  std::random_device rd_;
  std::mt19937 g_;

  std::vector<std::pair<int, int>> tile_positions_;
  std::vector<std::pair<std::pair<int, int>, RoadType>> road_positions_;
  std::vector<std::pair<int, int>> building_positions_;

  std::unique_ptr<ncpp::Plane> board_;
  std::unique_ptr<ncpp::Plane> water_border_;
  std::unique_ptr<ncpp::Plane> toolbar_;

  std::vector<std::unique_ptr<ncpp::Plane>> tiles_;
  std::vector<std::unique_ptr<ncpp::Plane>> numbers_;
  std::vector<std::unique_ptr<ncpp::Plane>> trades_;
  std::vector<std::unique_ptr<ncpp::Plane>> settles_;
  std::vector<std::unique_ptr<ncpp::Plane>> cities_;
  std::vector<std::unique_ptr<ncpp::Plane>> roads_;
  std::unique_ptr<ncpp::Plane> dice0_;
  std::unique_ptr<ncpp::Plane> dice1_;
  std::unique_ptr<ncpp::Plane> dice2_;

  unsigned int numbers_offset_;

  ncpp::Plane *stdplane_;
  std::atomic_bool &gameover_;
  std::chrono::milliseconds msdelay_;
  unsigned y_, x_;
  volatile bool board_drawn;
  static std::vector<int> bag_red;
  static std::vector<int> bag_yellow;
  static std::vector<int> bag_special;
};

#endif
