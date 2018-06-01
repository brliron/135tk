g++ main.cpp File.cpp TFPK.cpp Rsa.cpp \
	-I../util ../util/UString.cpp ../util/UString_unix.cpp \
	-Wall -Wextra \
	-IMIRACL MIRACL/miracl.a -lz -o th135arc-alt -g
