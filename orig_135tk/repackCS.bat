@echo off
echo Repacking csv..
for /r %%c in (*.csv) do if [%%~xc] == [.csv] TFCSTool /w "%%c" "%%c"
pause
