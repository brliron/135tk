git submodule init
git submodule update

cd th135arc-alt/MIRACL
zip -r miracl.zip *
unzip -j -aa -L -o miracl.zip
bash linux64
cd ../..

cd th145arc/MIRACL
zip -r miracl.zip *
unzip -j -aa -L -o miracl.zip
bash linux64
cd ../..
