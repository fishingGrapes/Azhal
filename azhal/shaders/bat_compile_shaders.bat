@ECHO OFF

echo "compiling spirv shader binaries..."

SET "DXC_ROOT=%~dp0..\..\tools\dxc\"
SET "SHADER_SRC_DIR=%~dp0\src\"

for %%f in (%SHADER_SRC_DIR%\*.vs) do (
echo "compiling %%~nf.vs" 
%DXC_ROOT%\dxc.exe -spirv -T vs_6_7 -E main %%f -Fo %%~nf.vspv
)

for %%f in (%SHADER_SRC_DIR%\*.ps) do (
echo "compiling %%~nf.ps" 
%DXC_ROOT%\dxc.exe -spirv -T ps_6_7 -E main %%f -Fo %%~nf.pspv
)

pause