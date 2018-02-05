@echo off
for /r %%c in (*.act) do ACT1Tool /w "%%c" "%%c.txt"
for /r %%c in (*.act) do del "%%c.txt"
for /r %%c in (*.act.txt_*.nut) do del "%%c"
pause
