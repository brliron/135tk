@echo off
for /r %%c in (*.nut) do cnutool.exe -d "%%c" "%%c.txt"
echo finished.
pause