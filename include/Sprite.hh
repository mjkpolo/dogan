#ifndef DOGAN_SPRITE
#define DOGAN_SPRITE

#include <array>
#include <stdint.h>
#include <unistd.h>

template <size_t rows, size_t cols> struct Sprite {
  const uint32_t pixels[rows][cols];
  const size_t rows_ = rows;
  const size_t cols_ = cols;
};

static Sprite<4, 4> blue_settlement = {
    .pixels =
        {
            {
                0x0,
                0xffa84f1b,
                0x0,
                0x0,
            },
            {
                0xff7e350a,
                0xffa84f1b,
                0xffff6003,
                0x0,
            },
            {
                0xff7e350a,
                0xffff6003,
                0xffff6003,
                0xffff6003,
            },
            {
                0x007e350a,
                0x00ff6003,
                0x00ff6003,
                0x00ff6003,
            },
        },
};

static Sprite<4, 4> orange_settlement = {
    .pixels =
        {
            {
                0x0,
                0xff1b4fa8,
                0x0,
                0x0,
            },
            {
                0xff0a357e,
                0xff1b4fa8,
                0xff0360ff,
                0x0,
            },
            {
                0xff0a357e,
                0xff0360ff,
                0xff0360ff,
                0xff0360ff,
            },
            {
                0xff0a357e,
                0xff0360ff,
                0xff0360ff,
                0xff0360ff,
            },
        },
};

#endif
