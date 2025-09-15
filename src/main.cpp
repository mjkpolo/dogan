#define NCPP_EXCEPTIONS_PLEASE
#include "dogan.hh"
#include "config.hh"
#include <atomic>
#include <chrono>
#include <iostream>
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
        d.cleanup_mode();
        if (mode == TOOLBAR_START) {
          d.set_mode(TOOLBAR_NONE);
        } else {
          d.set_mode(TOOLBAR_START);
        }
        d.DrawToolbar();
        nc.render();
        break;
      case 0x4d:
        d.cleanup_mode();
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
          for (int i = 0; i < 10; i++) {
            d.DrawDice(i==9);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            nc.render();
          }
          break;
        default:
          break;
        }
        break;
      default:
        // std::cout << "input=0x" << std::hex << input
        //     << " id=" << std::dec << ni.id
        //     << " ctrl=" << ni.ctrl
        //     << " alt=" << ni.alt
        //     << " shift=" << ni.shift
        //     << std::endl;

        break;
      }
    }
  }
  return gameover || input == 'q';
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_config_file.yaml>" << std::endl;
        return 1;
    }

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
    GameConfig config;
    config.load_config(argv[1]);
    Dogan d{nc, gameover};

    auto cfgplane = nc.get_stdplane();
    d.DrawPopup(2000, config.summary());
    // std::cout << config.summary() << std::endl;
    nc.render();
    // config.summary();
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
