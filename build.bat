@echo off
windres Roboban.rc -O coff -o Roboban.res
gcc source/*.c Roboban.res %~1 -DTITLE="\"Roboban\"" -mwindows -o Roboban.exe -lallegro_monolith -static-libgcc
pause
