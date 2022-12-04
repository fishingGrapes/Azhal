@echo OFF
SET "mypath=%~dp0bin"
if EXIST %mypath% RD /S /Q %mypath%

SET "mypath=%~dp0temp\int"
if EXIST %mypath% RD /S /Q %mypath%


