@echo off
echo Repacking pictures..
for /r %%c in (*.png) do if [%%~xc] == [.png] TFBMTool /w "%%c" "%%c"
for /r %%c in (*.bmp) do if [%%~xc] == [.bmp] TFBMTool /w "%%c" "%%c"

pause
