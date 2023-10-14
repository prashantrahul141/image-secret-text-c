#!/bin/sh
echo Building from source...
mkdir -p build
gcc main.c -o build/main -O3
