@echo off
rem Builds pcwin\ddraw.dll (32-bit) with MSVC.
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64_x86 >nul || exit /b 1
cd /d %~dp0
cl /nologo /O2 /W3 /MT /LD /D_CRT_SECURE_NO_WARNINGS ddraw.c /Fe:..\ddraw.dll /link /DEF:ddraw.def user32.lib gdi32.lib winmm.lib || exit /b 1
del ..\ddraw.exp ..\ddraw.lib ddraw.obj 2>nul
