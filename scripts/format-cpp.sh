#!/bin/sh
# helper script to run clang-format on everything

# NOTE: must be the same version used in CI
prog=clang-format-18

args="--dry-run" # default, just check if formatting is OK
[ -z "$1" ] || args="$@"

# run clang-format on selected files but
# not submodules or .git/ files
find . \
	-type f \
	-and '(' \
	  -name '*.cpp' \
	  -or -name '*.h' \
	  -or -name '*.c' \
	')' \
       	-and '(' -not -path './3rd-party/*' ')' \
       	-and '(' -not -path './.git/*' ')' \
	-exec $prog $args '{}' ';' 
