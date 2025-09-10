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

bool IOLoop(ncpp::NotCurses &nc, Dogan &d, std::atomic_bool &gameover) {
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
      toolbar_mode mode = d.get_mode();
      switch (input) {
      case 0x53:
        if (mode == TOOLBAR_START) {
          d.set_mode(TOOLBAR_NONE);
        } else {
          d.set_mode(TOOLBAR_START);
        }
        d.DrawToolbar();
        nc.render();
        break;
      case 0x4d:
        if (mode == TOOLBAR_MOVE) {
          d.set_mode(TOOLBAR_NONE);
        } else {
          d.set_mode(TOOLBAR_MOVE);
        }
        d.DrawToolbar();
        nc.render();
        break;
      case 'r':
        switch (mode) {
        case TOOLBAR_NONE:
          break;
        case TOOLBAR_START:
          break;
        case TOOLBAR_MOVE:
          stdplane->cursor_move(0, 0);
          stdplane->printf("FINNA ROLL THIS BITCH U+%06x", input);
          nc.render();
          break;
        default:
          break;
        }
        break;
      default:
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
