#!/usr/bin/env bash

# setup a ram disk for homebrew and compiling
set -e
diskutil eraseVolume HFS+ "Scratch" `hdiutil attach -nomount ram://$((2*4096*512))`

# below part fails in brew install, I guess we just rm -rf /usr/local instead
exit 0

# mount ramdisk on top of /usr/local for a clean environment
DISK=$(hdiutil attach -nomount ram://$((2*4096*512)) )
sudo newfs_hfs -v Homebrew $DISK
sudo mount -t hfs $DISK /usr/local
echo ramdisk $DISK mounted @ /usr/local
