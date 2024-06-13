#!/bin/bash
# Based on https://github.com/scrubbbbs/cbird/blob/main/windows/mxe-pkg.sh
#
# Builds a portable binary package for Windows by copying some files and
# pulling all linked .dlls automagically using wine
#
# Used by "make collect" on the mingw cmake target
#
# Requires environment variables:
#   MXE_DIR    => path to mxe installation (root of mxe git)
#   MXE_TARGET => name of mxe build target (x86_64-w64-mingw32.shared)
#
VERSION=$1                              # version in zip filename
QT=$2                                   # qt major version
DIST=$3                                 # dirname of misc build products (Debug|Release)
BUILD=.                                 # directory containing build products
ARCH=$(echo $MXE_TARGET | cut -d- -f1)  # arch in zip file name (x86_64)
PKG_DIR=$BUILD/nomacs-mingw             # output directory
ZIP=nomacs-mingw-$VERSION-$ARCH.zip     # zipped output directory

MXE_BIN="$MXE_DIR/usr/$MXE_TARGET/bin"  # mxe-compiled dlls
SELF_BIN=.                              # our own dlls
CROSS_BIN=libs-mxe                      # dlls we compiled separate from mxe
QT_DIR="$MXE_DIR/usr/$MXE_TARGET/qt$QT" # qt plugins
QT_BIN="$QT_DIR/bin"                    # qt dlls

echo "packaging $VERSION $ARCH Qt$QT in $PKG_DIR"

mkdir -p "$PKG_DIR"

# todo: copy the plugins we actually need
echo "copying qt plugins..."
for x in imageformats platforms printsupport styles; do
 cp -auv "$QT_DIR/plugins/$x" "$PKG_DIR/"
done

echo "copying themes..."
cp -auv "$BUILD/$DIST/themes" "$PKG_DIR/"

echo "copying translations..."
mkdir -p "$PKG_DIR/translations"
cp -auv *.qm "$PKG_DIR/translations"

# todo: run nomacs in such a way that all plugins try to load so we can find deps
echo "copying nomacs plugins..."
mkdir -p "$PKG_DIR/plugins"
cp -auv plugins/*/*.dll "$PKG_DIR/plugins/"

echo "copying programs..."
mkdir -p "$PKG_DIR"
cp -auv nomacs.exe "$PKG_DIR/"

# for some reason loop below won't pickup zlib
cp -auv "$MXE_DIR/usr/$MXE_TARGET/bin/zlib1.dll" "$PKG_DIR/"

# use wine to find the dlls and copy them to package dir
# loop repeats until no more errors appear
# note: there will be missing dlls if they are loaded dynamically, may
#       require running in a mode that will cause them all to load
for exe in nomacs.exe; do
    PASS=1
    while [ $PASS -ge 1 ]; do
        echo "copying dlls for $exe (pass $PASS) ..."
        LAST=$PASS
        PASS=0
        DLLS=`wine "$PKG_DIR/$exe" --version 2>&1 | grep :err:module:import_dll | cut -d' ' -f3`
        for x in $DLLS; do
            if   [ -e "$SELF_BIN/$x"   ]; then cp -au "$SELF_BIN/$x" "$PKG_DIR/"
            elif [ -e "$MXE_BIN/$x"    ]; then cp -au "$MXE_BIN/$x" "$PKG_DIR/"
            elif [ -e "$QT_BIN/$x"     ]; then cp -au "$QT_BIN/$x" "$PKG_DIR/"
            elif [ -e "$CROSS_BIN/$x"  ]; then cp -au "$CROSS_BIN/$x" "$PKG_DIR/"
            else
                echo "can't find dll: $x"
                exit 1
            fi
            PASS=$(($LAST + 1))
        done
    done

    echo "dependencies complete, cross your fingers and run 'wine $PKG_DIR/$exe'"
done

# make the zip file
#(cd "$PKG_DIR/.." && rm -fv "$ZIP" && zip -r "$ZIP" "$PKG_DIR")
