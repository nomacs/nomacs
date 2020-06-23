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

def increment(filepath: str):
    from shutil import move
    from os import remove


    outpath = filepath + "tmp"

    with open(filepath, "r") as src:
        with open(outpath, "w") as dst:
        
            for l in src.readlines():

                l = increment_version_string(l)
                l = increment_version(l)

                l = add_git_tag_string(l)

                dst.write(l)    

    remove(filepath)
    move(outpath, filepath)

def increment_version_string(line: str):

    # seraching: #define NOMACS_VERSION_STR "0.3.5.0\0"
    if "NOMACS_VERSION_STR" in line:
        
        line = line.replace("\n", "")
        v = line.split(".")

        build = v[-1].split("\"")

        if len(v) == 4:
            # increment
            build[0] = str(int(build[0])+1)
            v[-1] = "\"".join(build)
        elif len(v) == 3:
            v[-1] = build[0] + ".0" + build[1]
        

        line = ".".join(v) + "\n"

        print("[Version Incrementer] version updated: " + line.split("\"")[-2])

    return line

def increment_version(line: str):
    
    # seraching: #define NOMACS_VERSION_RC 0,3,5,0
    if "NOMACS_VERSION_RC" in line:

        line = line.replace("\n", "")
        v = line.split(",")

        if len(v) == 4:
            v[-1] = str(int(v[-1])+1)
        elif len(v) == 3:
            v[-1] += ",0"

        line = ",".join(v) + "\n"

    return line

def add_git_tag_string(line: str):

    # seraching: #define NOMACS_GIT_TAG "4add4f1f6b6c731a9f4cf63596e087d4f68c2aed"
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

    parser.add_argument("inputfile", type=str,
                        help="""full path to the project's DkVersion.h file""")

    args = parser.parse_args()

    if not os.path.isfile(args.inputfile):
        print("input file does not exist: " + args.inputfile)
        exit()

    increment(args.inputfile)
