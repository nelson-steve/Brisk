@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile SDFGI shading shaders
echo Compiling ClearGDF.comp...
glslc ClearGDF.comp -o Compiled/ClearGDF.spv || goto :error

:: Compile SDFGI shading shaders
echo Compiling GDFSplat.comp...
glslc GDFSplat.comp -o Compiled/GDFSplat.spv || goto :error

:: Compile SDFGI shading shaders
echo Compiling Voxelization.vert...
glslc Voxelization.vert -o Compiled/VoxelizationVS.spv || goto :error

:: Compile SDFGI shading shaders
echo Compiling Voxelization.geom...
glslc Voxelization.geom -o Compiled/VoxelizationGS.spv || goto :error

:: Compile SDFGI shading shaders
echo Compiling Voxelization.frag...
glslc Voxelization.frag -o Compiled/VoxelizationFS.spv || goto :error

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
