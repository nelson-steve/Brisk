@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile deferred renderer shaders
echo Compiling RayGen.rgen...
glslc RayGen.rgen -o Compiled/RayGen.spv --target-env=vulkan1.4 || goto :error

echo Compiling CHit.rchit...
glslc CHit.rchit -o Compiled/CHit.spv --target-env=vulkan1.4 || goto :error

echo Compiling Miss.rmiss...
glslc Miss.rmiss -o Compiled/Miss.spv --target-env=vulkan1.4 || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
