#!/bin/sh
g++ main.cpp TFPK.cpp FnList.cpp FilesList.cpp Rsa.cpp \
	-I../util ../util/os.cpp ../util/File.cpp ../util/UString.cpp \
	-Wall -Wextra \
	-IMIRACL MIRACL/miracl.a -lz -o th135arc-alt -g
