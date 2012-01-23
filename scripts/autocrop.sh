#!/bin/sh
#
# This scripts removes extra "blank" pixels around the main part of image.
#

(( $# == 2 )) || exit 1

declare -r input=$(cd $(dirname "$1") && pwd)/$(basename "$1")
declare -r output=$(cd $(dirname "$2") && pwd)/$(basename "$2")

[ ${input} != ${output} ] || exit 1

echo "Input: $(identify $1)"

convert ${input} -fuzz 10% -trim ${output}

echo "Output: $(identify $2)"
