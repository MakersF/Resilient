FROM ubuntu:xenial

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    software-properties-common \
    cmake \
    libgtest-dev \
    google-mock \
    libboost1.58-dev

RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y && apt-get update