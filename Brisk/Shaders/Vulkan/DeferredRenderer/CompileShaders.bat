@echo off
setlocal

if not exist "Compiled" mkdir Compiled

:: Compile shaders
echo Compiling Geometry.vert...
glslc Geometry.vert -o Compiled/GeometryVS.spv || goto :error

echo Compiling Geometry.frag...
glslc Geometry.frag -o Compiled/GeometryFS.spv || goto :error

echo Compiling Lighting.vert...
glslc Lighting.vert -o Compiled/LightingVS.spv || goto :error

echo Compiling Lighting.frag...
glslc Lighting.frag -o Compiled/LightingFS.spv || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
