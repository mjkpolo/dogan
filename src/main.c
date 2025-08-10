#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  setlocale(LC_ALL, "");
  const notcurses_options opts = {
      .termtype = NULL,
      .loglevel = NCLOGLEVEL_ERROR,
      .margin_t = 0,
      .margin_r = 0,
      .margin_b = 0,
      .margin_l = 0,
      .flags = 0,
  };
  struct notcurses *nc = notcurses_core_init(&opts, NULL);
  if (nc == NULL) {
    return EXIT_FAILURE;
  }
  unsigned dimy, dimx;
  struct ncplane *n = notcurses_stddim_yx(nc, &dimy, &dimx);

  nccell ul = {}, ll = {}, lr = {}, ur = {}, hl = {}, vl = {};
  nccells_rounded_box(n, 0, 0, &ul, &ur, &ll, &lr, &hl, &vl);

  ncplane_box(n, &ul, &ur, &ll, &lr, &hl, &vl, dimy - 1, dimx - 1, 0);
  const char *title = "Dogan";
  ncplane_printf_yx(n, dimy / 2 - 1, dimx / 2 - strlen(title) / 2, "Dogan");

  if (notcurses_render(nc)) {
    notcurses_stop(nc);
    return EXIT_FAILURE;
  }
  notcurses_get_blocking(nc, NULL);
  notcurses_stop(nc);
  return EXIT_SUCCESS;
}
