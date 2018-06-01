git submodule init
git submodule update

cd th145arc/MIRACL
zip miracl.zip *
unzip -j -aa -L -o miracl.zip
bash linux
cd ../..
