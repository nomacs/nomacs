#!/usr/bin/env bash

# compute a reasonable cache key for a set of homebrew formulae
# arguments: homebrew formula names separated by spaces

ARCH=$(uname -m)
MACOS_VERSION=$(sw_vers -productVersion | cut -d. -f1)

if [ ${MACOS_VERSION} -ne 15 ]; then
  echo "$0: unsupported macOS version: ${MACOS_VERSION}" >&2
  exit 1
fi

# jq filter on formulae[].bottle.stable.files to select
# the correct bottle for the current OS/Architecture 
BOTTLE_FILTER=''

ARCH=x86_64

if [ "${ARCH}" = "arm64" ]; then
  BOTTLE_FILTER='.arm64_sequoia // .all'

elif [ "${ARCH}" = "x86_64" ]; then
  # some 10.15/intel bottles are shared with 10.14
  BOTTLE_FILTER='.sequoia // .sonoma // .all'
else
  echo "$0: unsupported ARCH: ${ARCH}" >&2
  exit 2
fi

set -e

# brew deps will warn about uninstalled formulae; in practice this
# does not seem to matter, but could depend on the bottle selection
echo "$@" | xargs -n 1 echo > pkgs.txt
brew deps --full-name --union "$@" >> pkgs.txt

PKGS=$(cat pkgs.txt | sort | uniq | xargs)
echo "$0: computed deps: $PKGS" >&2

brew info --json=v2 $PKGS | jq "[ .formulae[] | {name: .name, vers: .versions.stable, src: (.urls.stable | (.revision // .checksum)), tap: .tap_git_head, formula: .ruby_source_checksum.sha256, bottle: ( (.bottle.stable.files) | (${BOTTLE_FILTER}) | .sha256 )} ]" > pkgs.info.txt

grep -w -B4 -A4 null pkgs.info.txt && \
  echo "$0: null data in brew info" >&2 && \
  exit 3

PKG_COUNT=$(echo $PKGS | xargs -n 1 echo | grep -c .)
INFO_COUNT=$(grep -c name pkgs.info.txt)
echo "$0:  $PKG_COUNT formulae" >&2
echo "$0:  $INFO_COUNT bottles for $BOTTLE_FILTER" >&2

if [ $PKG_COUNT -ne $INFO_COUNT ]; then
  echo "$0: package/bottle counts do not match" >&2
  exit 4
fi


cat pkgs.info.txt | sha256
