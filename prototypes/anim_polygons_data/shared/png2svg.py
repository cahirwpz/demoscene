#!/usr/bin/env python3

import os
import sys
import argparse
import xml.etree.ElementTree as ET
from PIL import Image, ImageOps

PASSEPARTOUT = 10
TMP_DIR = ""
TMP_SVG_FILE = "vtrace_tmp.svg"

xmlns = {"svg": "http://www.w3.org/2000/svg"}


def run_vtrace(img):
    return os.system("vtracer --colormode bw -c 30"
                     " --color_precision 8 --filter_speckle 15"
                     f" --gradient_step 0 --mode polygon --input {img}"
                     f" --output {os.path.join(TMP_DIR, TMP_SVG_FILE)}")


def make_passpepartout(img):
    img_name = os.path.join(TMP_DIR, "passepartout.png")
    ImageOps.expand(ImageOps.grayscale(Image.open(img)),
                    PASSEPARTOUT, 0).save(img_name, "png")
    return img_name


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Process rendered animation "
                                                 "into a single .svg file for "
                                                 "anim-polygons effect.")
    parser.add_argument("anim-dir", help="Directory containing blender "
                                         "animation output.", type=str)
    parser.add_argument("-o", "--output", metavar="OUT.svg",
                        help="Output svg file name.",
                        default="dancing.svg", type=str)
    args = parser.parse_args()
    ANIM_DIR = args.anim-dir
    SVG_FILE = args.output
    if os.path.isdir(ANIM_DIR) is False:
        sys.exit(f"anim-dir must be a valid directory!!")

    ET.register_namespace('', xmlns["svg"])
    first = True
    doc = None
    for frame_number, IN_IMG in enumerate(sorted(os.listdir(ANIM_DIR))):
        IN_IMG = make_passpepartout(os.path.join(ANIM_DIR, IN_IMG))
        if run_vtrace(IN_IMG):
            sys.exit(f"A problem occured on frame: {frame_number}")
        os.remove(IN_IMG)
        if first:
            first = False
            doc = ET.parse(os.path.join(TMP_DIR, TMP_SVG_FILE))
        else:
            tmp_doc = ET.parse(os.path.join(TMP_DIR, TMP_SVG_FILE)).getroot()
            doc.getroot().append(tmp_doc.find("svg:path", xmlns))
        doc.write(SVG_FILE)
    os.remove(TMP_SVG_FILE)
