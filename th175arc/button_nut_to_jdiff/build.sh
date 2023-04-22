#!/bin/sh
g++ button_nut_to_jdiff.cpp ../../Act-Nut-lib/libactnut.so -Wall -Wextra -Wno-multichar -I../../Act-Nut-lib/ -o button_nut_to_jdiff
cp ../../Act-Nut-lib/libactnut.so .
