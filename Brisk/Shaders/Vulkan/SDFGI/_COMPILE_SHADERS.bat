@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile SDFGI shading shaders
echo Compiling ClearGDF.comp...
glslc ClearGDF.comp -o Compiled/ClearGDF.spv || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
