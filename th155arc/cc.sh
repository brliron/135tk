/mingw32/bin/g++ th145arc.cpp TFPKArchive.cpp tasofroCrypt.cpp \
	-Wall -Wextra -Wno-multichar -Wno-parentheses -Wno-sign-compare -std=c++11 \
	-DUNICODE -D_UNICODE \
	-Iinclude -L. -lmiracl -static-libgcc -static-libstdc++ \
	-Wl,--enable-stdcall-fixup -o th145arc -m32 $*
