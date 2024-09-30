gcc bmpfont_extract.c -Wall -Wextra -o bmpfont_extract.exe
gcc bmpfont_convert.c -Wall -Wextra -o bmpfont_convert.exe

gcc bmpfont_create_gdi.c                        -Wall -Wextra -shared -lgdi32           -o bmpfont_create_gdi.dll
g++ bmpfont_create_gdiplus.cpp                  -Wall -Wextra -shared -lgdi32 -lgdiplus -o bmpfont_create_gdiplus.dll
gcc bmpfont_create_main.c bmpfont_create_core.c -Wall -Wextra         -lpng             -o bmpfont_create.exe
