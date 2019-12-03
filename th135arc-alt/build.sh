#!/bin/sh
g++ main.cpp TFPK.cpp FnList.cpp FilesList.cpp Rsa.cpp OS.cpp \
	-Wall -Wextra -std=c++2a -municode \
	-IMIRACL MIRACL/miracl.a -lz -o th135arc-alt -g
