#!/usr/bin/env python3

import os
import sys
import argparse
import xml.etree.ElementTree as ET
from PIL import Image, ImageOps

PASSEPARTOUT = 10
TMP_DIR = ""
TMP_SVG_FILE = "vtrace_tmp.svg"

XMLNS = {"svg": "http://www.w3.org/2000/svg"}


def run_vtrace(img):
    return os.system(
            "vtracer --colormode bw -c 30"
            " --color_precision 8 --filter_speckle 15"
            f" --gradient_step 0 --mode polygon --input {img}"
            f" --output {os.path.join(TMP_DIR, TMP_SVG_FILE)}")


def make_passpepartout(img):
    img_name = os.path.join(TMP_DIR, "passepartout.png")
    ImageOps.expand(
            ImageOps.grayscale(Image.open(img)),
            PASSEPARTOUT,
            0).save(img_name, "png")
    return img_name


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
            description="Process rendered animation into a single "
                        ".svg file for anim-polygons effect.")
    parser.add_argument(
            "anim_dir",
            help="Directory containing blender animation output.",
            type=str)
    parser.add_argument(
            "-o", "--output",
            metavar="OUT",
            help="Output svg file name.",
            default="dancing.svg",
            type=str)
    args = parser.parse_args()
    if not os.path.isdir(args.anim_dir):
        sys.exit(f"anim_dir must be a valid directory!!")

    ET.register_namespace('', XMLNS["svg"])
    first = True
    doc = None
    for frame_number, in_img in enumerate(sorted(os.listdir(args.anim_dir))):
        in_img = make_passpepartout(os.path.join(args.anim_dir, in_img))
        if run_vtrace(in_img):
            sys.exit(f"A problem occured on frame: {frame_number}")
        os.remove(in_img)
        if first:
            first = False
            doc = ET.parse(os.path.join(TMP_DIR, TMP_SVG_FILE))
        else:
            tmp_doc = ET.parse(os.path.join(TMP_DIR, TMP_SVG_FILE)).getroot()
            doc.getroot().append(tmp_doc.find("svg:path", XMLNS))
        doc.write(args.output)
    os.remove(TMP_SVG_FILE)
