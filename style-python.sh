#!/bin/sh

PYFILES=$(find a500 -name '*.py')
PYEXTRA="a500/effects/RunInUAE"

pycodestyle --ignore=E111,E114 ${PYEXTRA} ${PYFILES}
RES=$?

if ! [ $RES -eq 0 ]
then
    echo "Formatting incorrect for Python3 files!"
    echo "Please manually fix warnings and errors listed above."
else
    echo "Formatting correct for Python3 files."
fi

exit $RES
