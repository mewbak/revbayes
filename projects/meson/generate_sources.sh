#!/bin/sh

DIR=$1 

echo -n "Generating source list for $DIR ... "

SRC=../../src

cd $SRC/$DIR

echo "${DIR}_sources = files([" > meson.build
find . -name '*.cpp' |
    sed "s|^|'|" |
    sed "s|$|',|" |
    grep -v 'main.cpp' >> meson.build
find . -name '*.c' |
    sed "s|^|'|" |
    sed "s|$|',|" >> meson.build
truncate -s-2 meson.build
echo "])" >> meson.build
echo >> meson.build


echo "done."
