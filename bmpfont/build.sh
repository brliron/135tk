gcc bmpfont_extract.c -Wall -Wextra -o bmpfont_extract.exe
gcc bmpfont_create.c  -Wall -Wextra -o bmpfont_create.exe  -lgdi32
cp bmpfont_extract.exe ../bin/
cp bmpfont_create.exe  ../bin/
