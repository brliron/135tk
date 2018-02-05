@echo off
for /r %%c in (*.nut) do (
cnutool.exe -w "%%c" "%%c.txt"
del "%%c.txt"
)
echo finished.
pause