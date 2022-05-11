#!/usr/bin/env python3
import os
import argparse


def run_blender(scene, out_dir):
    os.system("xvfb-run --auto-servernum "
              "--server-args='-screen 0 1600x1024x16' "
              f"blender --background {scene} --render-output "
              f"{out_dir}/frame####.png --render-anim -noaudio")


def clean_dir(out_dir):
    for filename in os.listdir(out_dir):
        file_path = os.path.join(out_dir, filename)
        try:
            os.remove(file_path)
        except Exception as e:
            print(f"Cannot delete: {file_path} -> {e}")


if __name__ == "__main__":

    # Parse arguments:
    parser = argparse.ArgumentParser(description="Render animation from "
                                                 "given scene to output "
                                                 "directory.")
    parser.add_argument("scene", help="Blender .scene file "
                                      "which is going to be rendered.",
                                      type=str)
    parser.add_argument("anim-dir", help="Directory for blender to output "
                                         "animation. (WARNING! This script "
                                         "removes its contents.)",
                                         type=str)
    args = vars(parser.parse_args())
    SCENE_FILE = args["scene"]
    ANIM_DIR = args["anim-dir"]
    if os.path.isfile(SCENE_FILE) is False:
        raise Exception(f"No such file as: {SCENE_FILE}")
    if os.path.isdir(ANIM_DIR) is False:
        raise Exception(f"No such directory as: {ANIM_DIR}")
    clean_dir(ANIM_DIR)
    run_blender(SCENE_FILE, ANIM_DIR)
