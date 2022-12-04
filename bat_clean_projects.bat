@echo OFF
DEL /S /Q *.sln *.vcxproj *.vcxproj.filters *.vcxproj.user

SET "mypath=%~dp0temp\build"
if EXIST %mypath% RD /S /Q %mypath%
