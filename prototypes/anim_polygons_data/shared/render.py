#!/usr/bin/env python3

import os
import sys
import argparse
import shutil


def clean_dir(out_dir):
    for filename in os.listdir(out_dir):
        file_path = os.path.join(out_dir, filename)
        try:
            if os.path.isfile(file_path):
                os.remove(file_path)
            if os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print(f"Cannot delete: {file_path} -> {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Render animation from "
                                                 "given scene to output "
                                                 "directory.")
    parser.add_argument("scene", help="Blender .scene file "
                                      "which is going to be rendered.",
                                      type=str)
    parser.add_argument("anim_dir", help="Directory for blender to output "
                                         "animation. (WARNING! This script "
                                         "removes its contents.)",
                                         type=str)
    args = parser.parse_args()
    if not os.path.isfile(args.scene):
        sys.exit(f"No such file as: {args.scene}")
    if not os.path.isdir(args.anim_dir):
        sys.exit(f"No such directory as: {args.anim_dir}")
    clean_dir(args.anim_dir)
    os.system("xvfb-run --auto-servernum "
              "--server-args='-screen 0 1600x1024x16' "
              f"blender --background {args.scene} --render-output "
              f"{args.anim_dir}/frame####.png --render-anim -noaudio")
