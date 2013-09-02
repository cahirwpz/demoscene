#!/bin/sh

[ -z "$1" ] && exit 1

cat <<EOF
CallbackT Callbacks[] = {
EOF

sed -ne '/CALLBACK/s/CALLBACK\s*(\([^)]*\)).*/  {"\1", \&\1},/p' $1

cat <<EOF
  {NULL, NULL}
};
EOF
