#!/usr/bin/env python3

import os.path
import sys
import math

if __name__ == "__main__":
    movements = float(sys.argv[1])
    big_r = float(sys.argv[2])

    print("#define MOVEMENTS %d" % movements)
    print("")
    print(
        "typedef struct {\n\
            int x_cos[MOVEMENTS];\n\
            int y_sin[MOVEMENTS];\n\
        } CircleMovements;"
    )
    print("")

    circle_movements = [dict(), dict()]
    circle_movements[0] = {"x_cos": [], "y_sin": []}
    circle_movements[1] = {"x_cos": [], "y_sin": []}
    for i in range(int(movements)):
        progress = i / movements * 2 * math.pi
        circle_movements[0]["x_cos"].append(int(big_r * math.cos(progress)))
        circle_movements[0]["y_sin"].append(int(big_r * math.sin(progress)))
        circle_movements[1]["x_cos"].append(
            int(big_r * math.cos((progress + math.pi) % (2 * math.pi)))
        )
        circle_movements[1]["y_sin"].append(
            int(big_r * math.sin((progress + math.pi) % (2 * math.pi)))
        )

    print("static const CircleMovements circle_movements[2] = {")
    for i in range(2):
        print("{")
        print(".x_cos = {")
        print("  %s," % ", ".join(map(str, circle_movements[i]["x_cos"])))
        print("},")
        print(".y_sin = {")
        print("  %s," % ", ".join(map(str, circle_movements[i]["y_sin"])))
        print("}},")
    print("};")
