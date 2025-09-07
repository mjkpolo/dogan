#define NCPP_EXCEPTIONS_PLEASE
#include "dogan.hh"
#include <atomic>
#include <cstdlib>
#include <locale.h>
#include <mutex>
#include <stdio.h>
#include <thread>
#include <unistd.h>

std::mutex ncmtx;

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
  // ncopts.loglevel = NCLOGLEVEL_DEBUG;
  ncopts.loglevel = NCLOGLEVEL_SILENT;
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
