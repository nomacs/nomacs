#! /usr/bin/env bash

# this script runs just before nomacs and after
# all other AppImage bootstrap scripts

# prevent local libraries from preempting the AppImage versions
# this fixed a segfault on my system
export LD_LIBRARY_PATH=

# libheif hardcodes its plugins path, but provides this override
export LIBHEIF_PLUGIN_PATH="$this_dir/usr/lib/libheif/plugins"

# linuxdeploy-plugin-qt has a hook that breaks dark theme
# see: https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues/155
export QT_QPA_PLATFORMTHEME=

