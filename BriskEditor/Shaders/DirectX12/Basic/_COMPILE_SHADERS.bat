@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile Basic shading shaders
echo Compiling BasicShader_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/BasicShader_vert.cso BasicShader_vert.hlsl

echo Compiling BasicShader_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/BasicShader_frag.cso BasicShader_frag.hlsl

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b