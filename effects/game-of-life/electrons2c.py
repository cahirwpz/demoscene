from math import ceil, log
from PIL import Image
import argparse

# Converts image with electron positions to C file
# electron head needs to have palette index 2
# electron tail needs to have palette index 3
# background can have palette index 0 or 1

header = "#include <gfx.h>\n\n\
#ifndef ELECTRONS_T\n\
#define ELECTRONS_T\n\
typedef struct ElectronsT {\n\
  Point2D* points;\n\
  u_short count;\n\
} ElectronsT;\n\
#endif\n\n"

def get_positions(infile):
    with Image.open(infile) as im:
        if im.mode != 'P':
            raise SystemExit('Only palette images supported.')

        width, height = im.size
        colors = im.getextrema()[1] + 1
        depth = int(ceil(log(colors, 2)))

        if depth != 2:
            raise SystemExit('Image needs to have exactly 2-bit depth')

        px = im.load()
        heads, tails = [], []
        for x in range(width):
            for y in range(height):
                if px[x, y] == 2:
                    heads.append((x, y))
                elif px[x, y] == 3:
                    tails.append((x, y))

        return heads, tails

def generate_array(points, name):
    res = ""
    res += "static Point2D {}_points[{}] = {{\n".format(name, len(points))
    for x, y in points:
        res += "  (Point2D){{.x={}, .y={}}},\n".format(x, y)
    res += "};\n\n"
    res += "static ElectronsT {} = {{\n".format(name)
    res += "  .points = {}_points,\n".format(name)
    res += "  .count = {}\n".format(len(points))
    res += "};\n\n"
    return res

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Converts 1-bit PNG with electron positions to C file."
    )
    parser.add_argument("infile", help="Input PNG file")
    parser.add_argument("outfile", help="Output C file")
    parser.add_argument("--heads", default="heads",
                        help="Name of the heads array")
    parser.add_argument("--tails", default="tails", 
                        help="Name of the tails array")
    args = parser.parse_args()

    heads, tails = get_positions(args.infile)
    
    with open(args.outfile, "w") as out:
        out.write(header)
        out.write(generate_array(heads, args.heads))
        out.write(generate_array(tails, args.tails))