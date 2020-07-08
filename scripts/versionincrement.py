#!/usr/bin/env python
"""Increments the build version of a C++ projects.

This script increments the build version after every build of 
a Visual Stuido project. It manipulates the DkVersion.h file in order
to do so. The version number defined there is displayed in help and
used for the VS_VERSION_INFO in the *.rc file
"""
import logging
import os

__author__ = "Markus Diem"
__credits__ = ["Markus Diem"]
__license__ = "GPLv3"
__version__ = "0.2"
__maintainer__ = "Markus Diem"
__email__ = "markus@nomacs.org"
__status__ = "Production"


OUTPUT_NAME = "versionincrement"

def increment(line: str, newversion: str):

    str_ver = line.split(":")

    # searching version.cache: version:0.3.5
    if "version" in line and len(str_ver) == 2:

        old_v = str_ver[1].split(".")

        if len(old_v) == 3:
            # increment
            build = str(int(old_v[-1])+1)

        line = str_ver[0] + ":" + newversion + "." + build

        # send status message only once
        print("[Version Incrementer] " + line.replace("version:", "version updated: "))

    return line


def update_version(version: str):
    from shutil import move
    from os import remove

    from utils.fun import version_cache

    filepath = version_cache()
    tmppath = filepath + "tmp"

    if os.path.exists(filepath):
        with open(filepath, "r") as src:
            with open(tmppath, "w") as dst:
                for l in src.readlines():

                    l = increment(l, version)
                    dst.write(l)
    
        # swap
        remove(filepath)
        move(tmppath, filepath)

    else:
        with open(filepath, "w") as dst:
            dst.write("version:" + version)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
       description='Increments the version in a version cache file.')

    parser.add_argument("version", type=str,
                        help="""current nomacs version - typically optained from cmake""")

    args = parser.parse_args()

    update_version(args.version)
