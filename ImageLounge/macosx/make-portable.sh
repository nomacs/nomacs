#!/bin/bash
#*******************************************************************************************************
# make-portable.sh
# Created on:	08.04.2025
#
# nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
#
# Copyright (C) 2025 Scrubs <scrubbbbs@gmail.com>
#
# This file is part of nomacs.
#
# nomacs is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# nomacs is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#*******************************************************************************************************/

#
# This script makes a portable bundle for nomacs, it is a replacement for macdeployqt.
#
# The main purpose is to find dylib dependencies, copy them to the bundle, and
# make them usable from the new location.
#
# macdeployqt simulates what the linker does at runtime, this seems to be problematic
# to get right; current nomacs requires a hack to modify the import table of Qt library
#
# We can avoid simulation issues by executing the program and enabling logging in the linker.
# This gives us all of the dylibs used quickly and accurately.
#
# We only must run the program in a way that all dylibs will be loaded, or else
# we will not discover all run-time dependencies. If we know of any dylibs we are not able to
# find this way, we have the option to add them manually.
#
# This has the effect of macdeployqt but much faster (10-20x)
#
# inputs
BUNDLE_ID="org.nomacs.ImageLounge"
BUNDLE_SRC="nomacs.app"                      # bundle containing exe
EXE_SRC="$BUNDLE_SRC/Contents/MacOS/nomacs"  # exe to make portable
EXE_ARGS="--about"                           # args to exe to validate build and load all dylibs
PLUGINS_SRC="/usr/local/share/qt/plugins"    # qt plugins location

# arguments
if [ -n "$1" ]; then
  PLUGINS_SRC="$1"
fi

# outputs
BUNDLE_DST=portable/nomacs.app
EXE_DST="$BUNDLE_DST/Contents/MacOS/nomacs"
LIBS_RPATH="@executable_path/../Frameworks" # same as macdeployqt
LIBS_DST="$BUNDLE_DST/Contents/Frameworks"  # same as macdeployqt
PLUGINS_DST="$BUNDLE_DST/Contents/PlugIns"  # same as macdeployqt
TMPDIR=portable/

# modify library imports table of a given exe/dylib to remove references
# to non-system paths (/usr/local /opt etc)
rename_imports()
{
  local DYLIB="$1"
  local libName="$(basename $DYLIB)"
  local DYLIB_DEPS=$(otool -l "$DYLIB" | grep -A2 LC_LOAD_DYLIB | grep " name " | \
    grep -vE "/usr/lib|/System" | /usr/bin/cut -w -f3)
  local RPATHS=$(otool -l "$DYLIB" | grep -A2 LC_RPATH | grep " path " | /usr/bin/cut -w -f3)

  # build arguments for install_name_tool

  # we don't technically have to set this but it might avoid confusion with other copies,
  # otherwise it would be the original absolute path of the library
  local args="-id $BUNDLE_ID/$libName"

  # remove all rpaths, add $LIBS_RPATH back to executables (never dylibs)
  for RPATH in $RPATHS; do
    #echo "$libName: remove rpath $RPATH"
    args="$args -delete_rpath $RPATH"
  done

  # change imports to @rpath/library.dylib. Note must also add $LIBS_RPATH
  # to the rpath (LC_RPATH load command) of every executable for this to work
  # the first dep is itself
  for OLDNAME in $DYLIB_DEPS; do
    local oldLibName=$(basename $OLDNAME)
    if [ $(echo $OLDNAME | grep '.framework') ]; then
      oldLibName=$(echo $OLDNAME | sed -re 's#.*/(.*.framework/.*)#\1#')
    fi

    local newName="@rpath/$oldLibName"
    #echo "$libName: $OLDNAME => $newName"
    args="$args -change $OLDNAME $newName"
  done

  if [ -n "$args" ]; then
    install_name_tool $args $DYLIB
  fi
}

# copy all of an exe's dylib dependencies to bundle, and modify imports of
# and modify imports in itself and copied dylibs
process_exe()
{
  local SRC_EXE=$1
  local DST_EXE=$2
  local ARGS=$3

  # change imports table so it doesn't reference anything in /usr/local or /opt (arm)
  rename_imports "$DST_EXE"

  # this the critical step that allows libs to be found in the bundle
  install_name_tool -add_rpath "$LIBS_RPATH" "$DST_EXE"

  echo "probing $SRC_EXE"
  local LIBS_USED="$TMPDIR/libs.orig.txt"

  # get all libraries loaded by the program with dyld logging
  # this gets both the imported library path and resolved path;
  # we need both to create symlinks
  DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 DYLD_PRINT_LIBRARIES=1 DYLD_PRINT_RPATHS=1 DYLD_PRINT_SEARCHING=1 \
    "$SRC_EXE" $ARGS 2>&1 | grep -A1 'found: dylib-from-disk:' | grep -v '^--' | sed -n 'N;s/\n/ /p' | sed 's/"//g' > "$LIBS_USED" 2>&1
  
  # copy .dylib files from the build directory /usr/local and /opt/homebrew, excluding plugins (handled separately)
  cat "$LIBS_USED" | grep -E "^dyld.*($PWD|/usr/local/|/opt/homebrew).*\.dylib" | \
    grep -vE "/PlugIns|plugins/" | /usr/bin/cut -w -f4 -f7 | while read -r LINE; do

    local import=$(echo $LINE | /usr/bin/cut -w -f1)
    local resolved=$(echo $LINE | /usr/bin/cut -w -f2)
    local libName=$(basename $import)
    local resName=$(basename $resolved)

    if [ $resolved -nt $LIBS_DST/$resName ]; then # skip already seen libraries
      echo "adding $import"
      cp -a $resolved $LIBS_DST/
      chmod 755 $LIBS_DST/$resName
      if [ "$libName" != "$resName" ]; then
        ln -s $resName $LIBS_DST/$libName
      fi
      rename_imports $LIBS_DST/$libName
    fi
  done

  cat "$LIBS_USED" | grep ^dyld | grep -E "(/usr/local/|/opt/).*\.framework.*" | /usr/bin/cut -w -f4 -f7 | while read -r LINE; do
      #echo $LINE
      # resolve framework symlinks and get the .framework bundle path
      local framework=$(echo $LINE | /usr/bin/cut -w -f2)
      local bundle=$(readlink -f $framework | /usr/bin/cut -d/ -f1-8)
      local dirName=$(basename $bundle)
      local libName=$(echo $dirName | /usr/bin/cut -d. -f1)

      if [ $framework -nt $LIBS_DST/$dirName ]; then
        rsync -auv --no-owner --no-group --exclude "Headers" --exclude "*.prl" $bundle $LIBS_DST/
        rename_imports $LIBS_DST/$dirName/$libName
      fi
  done
}

test_exe()
{
  local CMD="$@"
  local LIBS_PATCHED="$TMPDIR/libs-patched.txt"
  echo "testing: $CMD"

  DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 DYLD_PRINT_LIBRARIES=1 DYLD_PRINT_RPATHS=1 \
      DYLD_LIBRARY_PATH= $CMD > "$LIBS_PATCHED" 2>&1

  local DYLIBS=$(cat "$LIBS_PATCHED" | grep ^dyld | grep -E "(/usr/local/|/opt)" | /usr/bin/cut -w -f3)

  local err=0
  for DYLIB in $DYLIBS; do
    echo "!! missing library !! $DYLIB"
    err=1
  done
  if [ $err -ne 0 ]; then exit 10; fi
  echo "SUCCESS: $CMD"
}

# program must work, we will be running it to find dependencies
"$EXE_SRC" $EXE_ARGS >/dev/null 2>&1
if [ $? -ne 0 ]; then
  echo the build is broken, check that $EXE_SRC is working
  exit 1
fi

# setup folders
mkdir -p "$BUNDLE_DST" || exit 2
rsync -auv "$BUNDLE_SRC/" "$BUNDLE_DST/" || exit 3
mkdir -pv "$PLUGINS_DST" "$LIBS_DST" || exit 4

# write qt.conf with the correct paths (matches macdeployqt)
echo -e '[Paths]\nPlugins = PlugIns\nImports = Resources/qml\nQmlImports = Resources/qml' \
  > $BUNDLE_DST/Contents/Resources/qt.conf || exit 5

# add exe and fix imports
process_exe "$EXE_SRC" "$EXE_DST" "$EXE_ARGS"

# copy Qt plugins. Some of these do not show up in process_exe, so
# we just ignore all of them there and add them manually here
# note: this is a default set of plugins matching macdeployqt
rsync -auv --no-owner --no-group --copy-links \
  "$PLUGINS_SRC/iconengines" \
  "$PLUGINS_SRC/imageformats" \
  "$PLUGINS_SRC/networkinformation" \
  "$PLUGINS_SRC/platforminputcontexts" \
  "$PLUGINS_SRC/platforms" \
  "$PLUGINS_SRC/styles" \
  "$PLUGINS_SRC/tls" \
  "$PLUGINS_DST/" || exit 6
DYLIBS=$(find "$PLUGINS_DST" -name '*.dylib')

echo "processing plugins"
for DYLIB in $DYLIBS; do
  rename_imports "$DYLIB"
done

# verify we do not use anything from /usr/local or /opt
test_exe "$EXE_DST" "$EXE_ARGS"
