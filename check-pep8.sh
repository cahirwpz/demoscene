#!/bin/sh

PYFILES=$(find a500 \( -name '*.py' -printf 'a500/%P\n' \))
PYEXTRA=""

pep8 --ignore=E111,W191,E121,E231 ${PYEXTRA} ${PYFILES}
RES=$?

if ! [ $RES -eq 0 ]
then
    echo "Formatting incorrect for Python3 files!"
    echo "Please manually fix warnings and errors listed above."
else
    echo "Formatting correct for Python3 files."
fi

exit $RES
