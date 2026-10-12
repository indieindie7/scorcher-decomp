@echo off
rem Scorcher (Windows 95 version) on Windows 11: our ddraw.dll draws it in a window and plays music\trackNN.wav as the CD music.
cd /d "%~dp0"
start "" SCORWIN.EXE l:\scorpc\ "%~dp0TAGDEN.BIN"
