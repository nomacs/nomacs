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

def get_version_from_file(filepath: str):

    from shutil import move
    from os import remove

    outpath = filepath + "tmp"

    with open(filepath, "r") as src:
        for l in src.readlines():

            version = get_version(l)

            if version:
                return version

    return ""


def get_version(line: str):

    # searching (DkVersion.h): #define NOMACS_VERSION_STR "0.3.5.0\0"
    if "NOMACS_VERSION_STR" in line:
        
        str_ver = line.split("\"")
        ver = str_ver[1].split(".")

        if len(ver) == 4:
            # increment
            ver[-1] = str(int(ver[-1])+1)
        elif len(ver) == 3:
            ver.append(".0")

        return ".".join(ver)
    else:
        return ""


def increment(versionfile: str, filepath: str):
    from shutil import move
    from os import remove

    v = get_version_from_file(versionfile)

    print("current version: " + v)

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
        line = str_ver[0] + "\"" + new_v + "\"" + str_ver[-1]

        # send status message only once
        if "NOMACS_VERSION_STR" in line:
            print("[Version Incrementer] version updated: " + line.split("\"")[-2])

    return line

def increment_version(new_v: str, line: str):
    
    # searching: #define NOMACS_VERSION_RC 0,3,5,0
    if "NOMACS_VERSION_RC" in line:

        str_ver = line.split(" ")
        str_ver[-1] = new_v.replace(".", ",")
 
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
                        help="""file path to the version config (i.e. DkVersion.h)""")

    parser.add_argument("inputfile", type=str,
                        help="""full path to the file who's version should be updated""")


    args = parser.parse_args()

    if not os.path.isfile(args.inputfile):
        print("input file does not exist: " + args.inputfile)
        exit()

    increment(args.versionfile, args.inputfile)
