#!/usr/bin/env python
"""Increments the build version of a C++ projects.

This script increments the build version after every build of 
a Visual Stuido project. It manipulates the DkVersion.h file in order
to do so. The version number defined there is displayed in help and
used for the VS_VERSION_INFO in the *.rc file
"""
import logging

__author__ = "Markus Diem"
__credits__ = ["Markus Diem"]
__license__ = "GPLv3"
__version__ = "0.3"
__maintainer__ = "Markus Diem"
__email__ = "markus@nomacs.org"
__status__ = "Production"


OUTPUT_NAME = "versionupdate"


def update(filepath: str, copy: bool = False):
    from shutil import move
    from os import remove
    from utils.fun import version

    v = version()

    dstpath, ext = os.path.splitext(filepath)
    dstpath += "-versioned" + ext

    with open(filepath, "r") as src:
        with open(dstpath, "w") as dst:
        
            for l in src.readlines():

                l = update_version_string(v, l)
                l = update_version_rc(v, l)
                l = update_version_patch(v, l)

                l = add_git_tag_string(l)

                dst.write(l)    

    # replace current file?
    if not copy:
        remove(filepath)
        move(dstpath, filepath)


def update_version_string(version: str, line: str):

    # searching (DkVersion.h): #define NOMACS_VERSION_STR "3.14.42\0"
    # searching (msi installer): <?define ProductVersion = "3.14.42"?>
    # searching (inno installer): define MyAppVersion "3.14.42"
    if "NOMACS_VERSION_STR" in line or \
        "<?define ProductVersion" in line or \
        "define MyAppVersion" in line:
        
        str_ver = line.split("\"")
        line = str_ver[0] + "\"" + version + "\"" + str_ver[-1]

    return line

def update_version_rc(version: str, line: str):
    
    # searching: #define NOMACS_VERSION_RC 3,14,42
    if "NOMACS_VERSION_RC" in line:

        str_ver = line.split(" ")
        str_ver[-1] = version.replace(".", ",")
 
        line = " ".join(str_ver) + "\n"

    return line

def update_version_patch(version: str, line: str):

    # get patch from 3.14.42
    vs = version.split(".")

    if len(vs) != 3:
        print("WARNING: could not split version: " + version)
        return

    # searching: #define NOMACS_VER_PATCH 0
    if "NOMACS_VER_PATCH" in line:

        str_ver = line.split(" ")
        str_ver[-1] = vs[-1]

        line = " ".join(str_ver) + "\n"

    return line

def add_git_tag_string(line: str):

    # searching: #define NOMACS_GIT_TAG "4add4f1f6b6c731a9f4cf63596e087d4f68c2aed"
    if "NOMACS_GIT_TAG" in line:

        line = line.replace("\n", "")

        v = line.split("\"")

        if len(v) == 3:
            v[-2] = git_tag()

        line = "\"".join(v) + "\n"

    return line

def git_tag():
    import subprocess

    tag = subprocess.check_output(["git", "rev-parse", "HEAD"])
    tag = tag.strip().decode("utf-8")

    return tag

if __name__ == "__main__":
    import argparse
    import os
    from utils.fun import mypath

    parser = argparse.ArgumentParser(
       description='Increments the build version of a C++ project and adds the git rev as product version.')


    parser.add_argument("inputfile", type=str,
                        help="""full path to the file who's version should be updated""")
    parser.add_argument("--copy", action='store_true',
                        help="""if set, a _ver file will be created""")

    args = parser.parse_args()

    if not os.path.isfile(args.inputfile):
        print("input file does not exist: " + args.inputfile)
        exit()

    update(args.inputfile, args.copy)
