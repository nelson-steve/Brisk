@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile clustered shading shaders
echo Compiling ClusterAABBGenerate.comp...
glslc ClusterAABBGenerate.comp -o Compiled/ClusterAABBGenerateCS.spv || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
