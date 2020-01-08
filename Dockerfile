FROM ubuntu:xenial

# fixes tzdata asking for timezone
ENV DEBIAN_FRONTEND=noninteractive

# update ...
RUN apt-get update && apt-get install -y --no-install-recommends apt-utils

RUN apt-get install -y \
    # make C++
    cmake \
    build-essential \
    # required packages to build nomacs
    qt5-default \
    qtcreator-dev \
    libqt5svg5-dev \
    libexiv2-dev \
    # optional packages
    libopencv-dev \
    libraw-dev \
    libquazip5-dev