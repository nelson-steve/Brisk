@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile deferred renderer shaders
echo Compiling RayGeneration.rgen...
glslc RayGeneration.rgen -o Compiled/RayGeneration.spv --target-env=vulkan1.4 || goto :error

echo Compiling ClosestHit.rchit...
glslc ClosestHit.rchit -o Compiled/ClosestHit.spv --target-env=vulkan1.4 || goto :error

echo Compiling Miss.rmiss...
glslc Miss.rmiss -o Compiled/Miss.spv --target-env=vulkan1.4 || goto :error

echo Compiling ShadowMiss.rmiss...
glslc ShadowMiss.rmiss -o Compiled/Shadow.spv --target-env=vulkan1.4 || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
