#!/usr/bin/python3

import argparse
import sys
import os.path
from contextlib import redirect_stdout
from math import sin, cos, pi


def gen_double_helix(radius=1.0, pitch=0.5, turns=2, points_per_turn=100):
    points = int(points_per_turn * turns)

    start = -pi * turns
    end = pi * turns
    vertices = []

    for i in range(points):
        theta = (end - start) * i / points + start

        x = radius * cos(theta)
        y = pitch * theta / (2.0 * pi)
        z = radius * sin(theta)
        vertices.append((x, y, z))

        x = radius * cos(theta + pi)
        y = pitch * theta / (2.0 * pi)
        z = radius * sin(theta + pi)
        vertices.append((x, y, z))

    return vertices


def gen_circular_double_helix(radius=2.0, helix_radius=0.5,
                              turns=2, points_per_turn=100):
    points = int(points_per_turn * turns)

    start = 0
    end = 2 * pi
    vertices = []

    for i in range(points):
        theta = (end - start) * i / points + start
        phi = turns * (end - start) * i / points + start

        x = (radius + helix_radius * cos(phi)) * cos(theta)
        y = (radius + helix_radius * sin(phi)) * sin(theta)
        z = helix_radius * sin(phi)
        vertices.append((x, y, z))

        phi += pi
        x = (radius + helix_radius * cos(phi)) * cos(theta)
        y = (radius + helix_radius * sin(phi)) * sin(theta)
        z = helix_radius * sin(phi)
        vertices.append((x, y, z))

    return vertices


def print_obj(name, vertices):
    print("# Double Helix OBJ file")
    print(f"mtllib {name}.mtl")
    print(f"o {name}")
    for x, y, z in vertices:
        x = round(x, 3)
        y = round(y, 3)
        z = round(z, 3)
        print(f"v {x} {y} {z}")
    print("usemtl default")
    print("g lines")
    for i in range(len(vertices) // 2):
        p0 = i * 2 + 1
        p1 = i * 2 + 2
        print(f"l {p0} {p1}")


def print_mtl(name):
    print("# Double Helix MTL file")
    print(f"newmtl default")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate Wavefront OBJ/MTL file with double helix.")
    parser.add_argument("--circular", action="store_true",
                        help="Generate circular double helix.")
    parser.add_argument("path", metavar="PATH", type=str,
                        help="Prefix of the target file.")
    args = parser.parse_args()

    name = os.path.basename(args.path)

    if args.circular:
        dna = gen_circular_double_helix(
                radius=2.5, helix_radius=0.5, turns=4.0, points_per_turn=10)
    else:
        dna = gen_double_helix(
                radius=1.5, pitch=10.0, turns=1.0, points_per_turn=16)

    with redirect_stdout(open(f"{args.path}.obj", "w")):
        print_obj(name, dna)

    with redirect_stdout(open(f"{args.path}.mtl", "w")):
        print_mtl(name)
