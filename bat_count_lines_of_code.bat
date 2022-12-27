@echo OFF
SET "COMMON_DIR=%~dp0common\src"
SET "AZHAL_DIR=%~dp0azhal\src"
SET "SANDBOX_DIR=%~dp0sandbox\src"

echo Project: Common
tools\cloc\cloc.exe %COMMON_DIR%
echo:

echo Project: Azhal
tools\cloc\cloc.exe %AZHAL_DIR%
echo:

echo Project: Sandbox
tools\cloc\cloc.exe %SANDBOX_DIR%
echo:

PAUSE