@echo off
echo Extracting pictures..
for /r %%c in (*.png) do if [%%~xc] == [.png] TFBMTool /d "%%c" "%%c"
for /r %%c in (*.bmp) do if [%%~xc] == [.bmp] TFBMTool /d "%%c" "%%c"

pause
