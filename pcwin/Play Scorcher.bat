@echo off
rem Scorcher (Windows 95 version) on Windows 11: our ddraw.dll draws it in a window and plays music\trackNN.wav as the CD music.
rem TAGDEN.BIN is passed without quotes: the 1994 exe keeps quote marks in its arguments ("file open error").
cd /d "%~dp0"
set SCORCHER_DDLOG=1
start "" SCORWIN.EXE l:\scorpc\ TAGDEN.BIN
