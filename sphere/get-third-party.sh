#!/bin/sh

die() {
    echo "Aborting..."
    exit 1
}

MIRROR=umn.dl.sourceforge.net
MSVC6_DEP=third-party-msvc-2003.01.25.zip
MINGW_DEP=third-party-mingw-2003.01.25.zip
TMP=third-party-tmp

mkdir -p $TMP || die
cd $TMP || die

wget http://$MIRROR/sourceforge/sphere/$MSVC6_DEP || die
wget http://$MIRROR/sourceforge/sphere/$MINGW_DEP || die

cd .. || die

unzip $TMP/$MSVC6_DEP || die
unzip $TMP/$MINGW_DEP || die

echo "Third-party dependencies downloaded..."
echo "You may now remove $TMP."
