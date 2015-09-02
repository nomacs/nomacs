#!/bin/bash

##
# this is called by the CMake build to build the docs
##
if [ $# == 1 ]; then
    CMAKE_BINARY_DIR="$1"
    cd ..
    
    if [ -e src/exv_conf.h ]; then
        mv src/exv_conf.h src/exv_conf.h.keep
    fi
    make config
    ./configure
    if [ -e src/exv_conf.h.keep ]; then
        mv src/exv_conf.h.keep src/exv_conf.h
    fi
    mkdir -p  src/bin/
    cp    -f  "${CMAKE_BINARY_DIR}/bin/taglist" src/bin/
    make doc
fi

# That's all Folks
##
