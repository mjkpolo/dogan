#define NCPP_EXCEPTIONS_PLEASE
#include <locale.h>
#include <ncpp/NotCurses.hh>
#include <ncpp/Visual.hh>
#include <stdio.h>
#include <thread>
#include <unistd.h>

static std::mutex ncmtx;

class Dogan {
public:
  Dogan(ncpp::NotCurses &nc, std::atomic_bool &gameover)
      : nc_(nc), stdplane_(nc_.get_stdplane()), gameover_(gameover),
        msdelay_(1000) {
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
    int tmpx = x / 2;
    int tmpy = y / 2;
    DrawTile(0x00b040, tmpx-12, tmpy);
    DrawTile(0xb04000, tmpx-12+6, tmpy+2);
    DrawTile(0x0400b0, tmpx-12+6, tmpy-2);
    DrawTile(0x00b040, tmpx, tmpy);
    DrawTile(0xb04000, tmpx+6, tmpy+2);
    DrawTile(0x0400b0, tmpx+6, tmpy-2);
  }

  void DrawTile(unsigned int rgb, int x, int y) {
    const size_t cols = 7;
    const size_t rows = 4;
    const char *texture = " a***b "
                          "a**9**b"
                          "c*****d"
                          " c***d ";
    auto tile = std::make_unique<ncpp::Plane>(rows, cols, y, x, nullptr);
    uint64_t channels = 0;
    ncchannels_set_bg_alpha(&channels, NCALPHA_TRANSPARENT);
    ncchannels_set_fg_alpha(&channels, NCALPHA_TRANSPARENT);
    tile->set_fg_rgb(rgb);
    tile->set_bg_alpha(NCALPHA_TRANSPARENT);
    tile->set_base("", 0, channels);
    y = 0;
    x = 0;
    for (size_t i = 0; i < strlen(texture); i++) {
      if (texture[i] == '*') {
        tile->putstr(y, x, "█");
      } else if (texture[i] == 'a') {
        tile->putstr(y, x, "▟");
      } else if (texture[i] == 'b') {
        tile->putstr(y, x, "▙");
      } else if (texture[i] == 'c') {
        tile->putstr(y, x, "▜");
      } else if (texture[i] == 'd') {
        tile->putstr(y, x, "▛");
      } else if (texture[i] != ' ') {
        tile->putc(y, x, texture[i]);
      }
      y += ((x = ((x + 1) % cols)) == 0);
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
