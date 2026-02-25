#!/bin/bash

# Generate makefile 
cmake -S . -B build

# Compile project using generated makefile 
make -f Makefile --directory=build
