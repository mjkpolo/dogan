#include <notcurses/notcurses.h>
#include <stdio.h>
#include <assert.h>

int main() {
  const notcurses_options opts = {
      .termtype = NULL,
      .loglevel = NCLOGLEVEL_WARNING,
      .margin_t = 0,
      .margin_r = 0,
      .margin_b = 0,
      .margin_l = 0,
      .flags = 0,
  };
  struct notcurses *state = notcurses_init(&opts, NULL);
  assert(state != NULL);
  return 0;
}
