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
__version__ = "0.1"
__maintainer__ = "Markus Diem"
__email__ = "markus@nomacs.org"
__status__ = "Production"


OUTPUT_NAME = "version-incrementer"

def increment(v: str, filepath: str):
    from shutil import move
    from os import remove


    outpath = filepath + "tmp"

    with open(filepath, "r") as src:
        with open(outpath, "w") as dst:
        
            for l in src.readlines():

                l = increment_version_string(v, l)
                l = increment_version(v, l)

                l = add_git_tag_string(v, l)

                dst.write(l)    

    remove(filepath)
    move(outpath, filepath)


def increment_version_string(new_v: str, line: str):

    # searching (DkVersion.h): #define NOMACS_VERSION_STR "0.3.5.0\0"
    # searching (msi installer): <?define ProductVersion = "3.14.42"?>
    # searching (inno installer): define MyAppVersion "3.14.5"
    if "NOMACS_VERSION_STR" in line or \
        "<?define ProductVersion" in line or \
        "define MyAppVersion" in line:
        
        str_ver = line.split("\"")
        old_v = str_ver[1].split(".")

        if len(old_v) == 4:
            # increment
            build = str(int(old_v[-1])+1)
        elif len(old_v) == 3:
            build = "0"

        line = str_ver[0] + "\"" + new_v + "." + build + "\"" + str_ver[-1]

        # send status message only once
        if "NOMACS_VERSION_STR" in line:
            print("[Version Incrementer] version updated: " + line.split("\"")[-2])

    return line

def increment_version(new_v: str, line: str):
    
    # searching: #define NOMACS_VERSION_RC 0,3,5,0
    if "NOMACS_VERSION_RC" in line:

        str_ver = line.split(" ")
        v = str_ver[-1].split(",")

        if len(v) == 4:
            build = str(int(v[-1])+1)
        elif len(v) == 3:
            build += ",0"
        else:
            print("cannot increment: " + line)
            return line

        new_v = new_v.replace(".", ",")
        str_ver[-1] = new_v + "," + build
 
        line = " ".join(str_ver) + "\n"

    return line

def add_git_tag_string(version: str, line: str):

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

    parser = argparse.ArgumentParser(
       description='Increments the build version of a C++ project and adds the git rev as product version.')

    parser.add_argument("version", type=str,
                        help="""current version""")

    parser.add_argument("inputfile", type=str,
                        help="""full path to the file""")


    args = parser.parse_args()

    if not os.path.isfile(args.inputfile):
        print("input file does not exist: " + args.inputfile)
        exit()

    increment(args.version, args.inputfile)
