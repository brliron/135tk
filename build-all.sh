# Build
make -C Act-Nut-Lib clean
make -C Act-Nut-Lib
cd bmpfont      ; ./build.sh; cd ..
cd nhtextool    ; ./build.sh; cd ..
cd read_pat     ; ./build.sh; cd ..
cd TFBMTool-alt ; ./build.sh; cd ..
cd th145arc     ; ./build.sh; cd ..

# Copy binaries
mkdir -p bin
cp Act-Nut-lib/libactnut.dll Act-Nut-lib/print-act-nut.exe bin
cp bmpfont/bmpfont_extract.exe bmpfont/bmpfont_convert.exe bmpfont/bmpfont_create.exe bmpfont/bmpfont_create_gdi.dll bmpfont/bmpfont_create_gdiplus.dll bin
cp nhtextool/nhtextool.exe bin
cp orig_135tk/* bin
cp read_pat/read_pat.exe bin
cp TFBMTool-alt/TFBMTool-alt.exe bin
cp th145arc/th145arc.exe th145arc/fileslist.txt bin

# Copy dependencies
DEP_DIR=/mingw32/bin
cp $DEP_DIR/libgcc_s_dw2-1.dll $DEP_DIR/libjansson-4.dll $DEP_DIR/libpng16-16.dll $DEP_DIR/libstdc++-6.dll $DEP_DIR/libwinpthread-1.dll $DEP_DIR/zlib1.dll bin

cp readme.md bin
