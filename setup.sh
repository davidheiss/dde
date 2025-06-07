#!/bin/bash

export CC=gcc

set -e
rm -rf build
# meson setup --prefix=/usr --buildtype debug build
# meson setup --buildtype debug build
meson setup --buildtype release build
