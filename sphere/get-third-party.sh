#!/bin/sh

die() {
    echo "Aborting..."
    exit 1
}

ADDRESS=sphere.sourceforge.net/flik/files
MSVC6_DEP=third-party-msvc.zip
MINGW_DEP=third-party-mingw.zip
TMP=third-party-tmp

mkdir -p $TMP || die
cd $TMP || die

wget http://$ADDRESS/$MSVC6_DEP || die
wget http://$ADDRESS/$MINGW_DEP || die

cd .. || die

unzip $TMP/$MSVC6_DEP || die
unzip $TMP/$MINGW_DEP || die

echo "Third-party dependencies downloaded..."
echo "You may now remove $TMP."
