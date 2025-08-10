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
        msdelay_(1000) {}

  void Ticker() { // FIXME ideally this would be called from constructor :/
    std::chrono::milliseconds ms;
    do {
      mtx_.lock();
      ms = msdelay_;
      mtx_.unlock();
      std::this_thread::sleep_for(ms);
      ncmtx.lock();
      // if (MoveDown()) {
      //   gameover_ = true;
      //   ncmtx.unlock();
      //   return;
      // }
      ncmtx.unlock();
    } while (!gameover_);
  }

private:
  ncpp::NotCurses &nc_;
  uint64_t score_;
  std::mutex mtx_; // guards msdelay_
  std::unique_ptr<ncpp::Plane> board_;
  ncpp::Plane *stdplane_;
  std::atomic_bool &gameover_;
  std::chrono::milliseconds msdelay_;
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
