#!/bin/sh

die() {
    echo "Aborting..."
    exit 1;
}

BUILD=build/vc6
INSTALL=installer
DIRS="docs games startup system util"

echo "Building Sphere..."
scons compiler=vc6 opt=1 || die

echo "Cleaning old files..."
rm -rf $INSTALL/*.exe || die
rm -rf $INSTALL/*.dll || die
for d in $DIRS; do
    rm -rf $INSTALL/$d || die
done

echo "Copying files from $BUILD to $INSTALL..."
cp -R $BUILD/*.exe $INSTALL || die
cp -R $BUILD/*.dll $INSTALL || die
for d in $DIRS; do
    cp -R $BUILD/$d $INSTALL || die
done

echo "Removing unnecessary generated files..."
(find $INSTALL -name .sconsign | xargs rm -f) || die

echo "Building installer..."
(cd $INSTALL && cygstart -a "compile-bz2" sphere.nsi) || die

echo "Done"
