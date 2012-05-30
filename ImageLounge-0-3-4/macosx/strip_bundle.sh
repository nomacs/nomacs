#!/bin/bash

#
# Usage: strip_bundle.sh path/to/directory arch-to-keep
# Example: strip_bundle.sh ./release/TOra.app x86_64
#    will provide x86_64 only content of the MacOS directory in the same place
#

echo "Generating new single-architecture bundle for arch: $2"
echo "Into: $1.$2 (rename required)"

ditto --rsrc --arch $2 $1 $1.$2

