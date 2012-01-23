#!/bin/sh
#
# For non-png files converts them to png format. For png files rewrites them,
# which involves correcting the number of used colors and compression scheme.
#

(( $# == 1 )) || exit 1

declare -r path=$(cd $(dirname $1) && pwd)
declare -r base=$(basename $1)

declare -r input=${path}/${base}
declare -r output=${path}/${base%\.*}.png
declare -r tempfile=$(mktemp).png

identify ${input}

if convert ${input} ${tempfile}; then
  mv ${tempfile} ${output}

  identify ${output}

  [ ${input} != ${output} ] && rm -i ${input}
fi
