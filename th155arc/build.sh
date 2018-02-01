g++ th155arc.cpp TFPKArchive.cpp tasofroCrypt.cpp \
	-Wall -Wextra -Wno-multichar -Wno-parentheses -Wno-sign-compare -std=gnu++11 \
	-DUNICODE -D_UNICODE \
	-IMIRACL MIRACL/miracl.a -o th155arc
