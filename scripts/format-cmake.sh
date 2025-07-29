#!/bin/bash
# run this script in repository root to format cmakefiles using gersemi
# - with no arguments check/modify all files
# - with arguments, forward them to gersemi
# - in workflows use "--quiet --check --diff"
# - check linux workflow (linux.yml) to get the correct version of gersemi
#
# setup:
# python -m venv ~/.gersemi
# source ~/.gersemi/bin/activate
# pip install gersemi==0.21.0
#

args="--in-place"
[ -z "$1" ] || args="$@"

abort() { echo -e "\nformat-cmake: $1\n" >&2; exit 1; }

cd ImageLounge || abort "you must run this script from the repository root"

gersemi \
    --definitions cmake/Utils.cmake plugins/cmake/Utils.cmake \
    --no-cache \
    $args \
    . cmake src

[ $? -eq 0 ] || abort "formatting errors found, run '$0' to format sources or correct manually"
