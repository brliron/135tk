@echo off
echo Extracting csv..
for /r %%c in (*.csv) do if [%%~xc] == [.csv] TFCSTool /d "%%c" "%%c"
pause