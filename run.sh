#!/bin/bash

make

# Histogram bins min max data
mpiexec -n 2 ./Histogram 10 0.0 5.0 100