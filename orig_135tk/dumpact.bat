@echo off
for /r %%c in (*.act) do ACT1Tool /d "%%c" "%%c.txt"
pause
