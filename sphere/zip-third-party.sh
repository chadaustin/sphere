#!/bin/sh

die() {
    echo "Aborting..."
    exit 1
}

if test "$1" = ""; then
  echo "usage: zip-third-party.sh <date>"
  exit 1
fi

zip -r third-party-mingw-$1.zip third-party-mingw  || die
zip -r third-party-msvc-$1.zip third-party-msvc    || die
