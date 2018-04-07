# Clean
make -C Act-Nut-Lib clean
rm -rf 135tk

# Build
make -C Act-Nut-Lib
for dir in bmpfont nhtextool read_pat TFBMTool-alt th145arc; do
    echo "Building $dir..."
    cd $dir
    ./build.sh
    cd ..
done

# Copy binaries
mkdir -p 135tk
cp Act-Nut-lib/libactnut.dll Act-Nut-lib/print-act-nut.exe 135tk
cp bmpfont/bmpfont_extract.exe bmpfont/bmpfont_convert.exe bmpfont/bmpfont_create.exe bmpfont/bmpfont_create_gdi.dll bmpfont/bmpfont_create_gdiplus.dll 135tk
cp nhtextool/nhtextool.exe 135tk
cp orig_135tk/* 135tk
cp read_pat/read_pat.exe 135tk
cp TFBMTool-alt/TFBMTool-alt.exe TFBMTool-alt/extractBM-alt.exe 135tk
cp th145arc/th145arc.exe th145arc/fileslist.txt 135tk

# Copy dependencies
DEP_DIR=/mingw32/bin
cp $DEP_DIR/libgcc_s_dw2-1.dll $DEP_DIR/libjansson-4.dll $DEP_DIR/libpng16-16.dll $DEP_DIR/libstdc++-6.dll $DEP_DIR/libwinpthread-1.dll $DEP_DIR/zlib1.dll 135tk

cp readme.md 135tk
