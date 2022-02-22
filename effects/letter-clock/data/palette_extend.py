#!/usr/bin/env python3

# File format of an extended palette is as follows:
# first line contains bitmasks of lenses that
# will be applied to the columns below.
# Then each line should contain:
# unique base colour in the first column that coresponds
# to a colour in the image file;
# in the second column a colour by which the first one will be replaced
# in the final palette (can be the same);
# then each column should contain a colour which will be matched
# left to right to the masks (lenses) supplied in the first line
# in order to put the colour into a matching position.

from PIL import Image
import argparse


def read_palette(im, max_colours):
    if im.mode != "P":
        raise SystemExit("Only 8-bit images with palette supported.")

    pal = im.getpalette()
    colours = im.getextrema()[1] + 1

    if pal is None:
        raise SystemExit("Image has no palette!")
    if colours > max_colours:
        raise SystemExit(
            "Image has {} colours, expected at most {}!".format(
                colours, max_colours
            )
        )

    r_pal = [num >> 4 for num in pal]
    r_cmap = [r_pal[i * 3 : (i + 1) * 3] for i in range(colours)]

    return r_cmap


def extend_palette(orig_cmap, path, max_colours):
    assert len(orig_cmap) == len(
        set([tuple(l) for l in orig_cmap])
    ), "elements in cmap must be unique"
    # We fill any possible holes in a new cmap with the background colour.
    # This way we don't need to enforce complete palettes in txt file.
    new_cmap = orig_cmap + [orig_cmap[0]] * (max_colours - len(orig_cmap))
    with open(path) as f:
        lens_tab = [int(lens, base=2) for lens in f.readline().split()]
        for line in f:
            colour_line = [
                [
                    (int(colour, base=16) & 3840) >> 8,
                    (int(colour, base=16) & 240) >> 4,
                    int(colour, base=16) & 15,
                ]
                for colour in line.split()
            ]
            base_colour = colour_line[0]
            # Should the base colour be replaced (eg. to make it invisible without lens),
            # put another one in the second column. Otherwise repeat the base colour.
            base_colour_replacement = colour_line[1]
            new_cmap[orig_cmap.index(base_colour)] = base_colour_replacement
            for index, colour in enumerate(colour_line[2:]):
                new_cmap[
                    orig_cmap.index(base_colour) | lens_tab[index]
                ] = colour
    return new_cmap


def save_palette(name, r_cmap):
    print("static const PaletteT %s = {" % name)
    print("  .count = %d," % len(r_cmap))
    print("  .colors = {")
    for r, g, b in r_cmap:
        print("    0x%x%x%x," % (r, g, b))
    print("  }")
    print("};")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Converts an image to bitmap extended with data from txt file."
    )
    parser.add_argument("--name", type=str, help="Name of the palette.")
    parser.add_argument(
        "--max_colours",
        type=int,
        help="Maximum number of colours in a palette.",
    )
    parser.add_argument(
        "--image_path", type=str, help="Path to the original image."
    )
    parser.add_argument(
        "--extended_palette",
        type=str,
        help="Path to txt file with the additional colours.",
    )
    args = parser.parse_args()

    cmap = read_palette(Image.open(args.image_path), args.max_colours)

    if args.extended_palette:
        cmap = extend_palette(cmap, args.extended_palette, args.max_colours)

    save_palette(args.name, cmap)
