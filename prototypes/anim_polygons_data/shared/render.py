#!/usr/bin/env python3

import os
import sys
import argparse
import shutil


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description="Render animation from given "
                        "scene to output directory.")
    parser.add_argument(
            "scene",
            help="Blender .scene file which is going to be rendered.",
            type=str)
    parser.add_argument(
            "anim_dir",
            help="Directory for blender to output animation. "
            "(WARNING! anim_dir will be removed first if it already exists.",
            type=str)
    args = parser.parse_args()
    if not os.path.isfile(args.scene):
        sys.exit(f"No such file as: {args.scene}")
    shutil.rmtree(args.anim_dir, ignore_errors=True)
    os.makedirs(args.anim_dir)
    os.system(
            "xvfb-run --auto-servernum "
            "--server-args='-screen 0 1600x1024x16' "
            f"blender --background {args.scene} --render-output "
            f"{args.anim_dir}/frame####.png --render-anim -noaudio")
