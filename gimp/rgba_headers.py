#!/usr/bin/env python3

from PIL import Image
from os.path import basename
from argparse import ArgumentParser
import numpy as np


def main(images, output_dir):
    for fn in images:
        name = basename(fn)
        assert name.endswith('.png')
        name = name[:-4]
        img = Image.open(fn)
        pixels = img.tobytes()
        print("Strides:", np.array(img).strides)
        assert len(pixels) % 4 == 0
        with open(f'{output_dir}/{name}.hh', 'w') as fp:
            fp.write(f"#ifndef DOGAN_{name.upper()}_SPRITE\n")
            fp.write(f"#define DOGAN_{name.upper()}_SPRITE\n")
            fp.write("#include <stdint.h>\n")
            fp.write(f"constexpr uint32_t {
                     name}_sprite[{len(pixels) // 4}] = {{\n")
            for i in range(len(pixels) // 4):
                r, g, b, a = pixels[4*i:4*(i+1)]
                fp.write(f'{hex((a << 24) | (b << 16) | (g << 8) | r)},\n')
            fp.write("};\n")
            fp.write("#endif\n")


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        'images',
        type=str,
        nargs='+',
    )
    parser.add_argument(
        '--output_dir',
        type=str,
        default='.',
    )
    args = parser.parse_args()
    main(args.images, args.output_dir)
