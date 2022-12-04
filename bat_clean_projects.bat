@echo OFF
DEL /Q *.sln

SET "mypath=%~dp0temp\build"
if EXIST %mypath% RD /S /Q %mypath%
