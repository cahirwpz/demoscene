#!/bin/bash

CMD=$1
SCENE=$2
OUTPUT=${SCENE%.blend}.svg 

shift 2

if [ -e "${SCENE}.opts" ]; then
  source "${SCENE}.opts"
else
  echo "error: \"${SCENE}.opts\" file does not exists"
  exit 1
fi

case "$CMD" in
  run)
    unxz --keep ${SCENE}.xz
    test -d anim || mkdir anim
    rm -f anim/frame*.{png,svg}

    if [ -z "$DISPLAY" ]; then
      xvfb-run --auto-servernum --server-args='-screen 0 1600x1024x16' \
        blender --background ${SCENE} --render-format PNG \
          --render-output 'anim/frame####.png' --render-anim -noaudio
    else
      blender --background ${SCENE} --render-format PNG \
        --render-output 'anim/frame####.png' --render-anim -noaudio
    fi

    for f in anim/frame*.png; do
      echo "$f -> ${f%.png}.svg"
      convert $f ${CONVERT_OPTS} $f
      vtracer ${VTRACER_OPTS} --input $f --output ${f%.png}.svg
    done

    python3 svg-anim.py ${SVGANIM_OPTS} --output ${OUTPUT} anim/
    ;;
  clean)
    rm -f ${OUTPUT} ${SCENE}
    rm -rf anim/
    ;;
  *)
    echo "error: unknown command \"$CMD\""
    exit 1
    ;;
esac
