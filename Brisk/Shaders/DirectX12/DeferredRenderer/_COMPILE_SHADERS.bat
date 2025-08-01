@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile Deferred renderer shaders
echo Compiling BasicShader_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/DepthPrePass_vert.cso DepthPrePass_vert.hlsl

echo Compiling BasicShader_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/DepthPrePass_frag.cso DepthPrePass_frag.hlsl

echo Compiling GeometryPass_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/GeometryPass_vert.cso GeometryPass_vert.hlsl

echo Compiling GeometryPass_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/GeometryPass_frag.cso GeometryPass_frag.hlsl

echo Compiling LightingPass_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/LightingPass_vert.cso LightingPass_vert.hlsl

echo Compiling GeometryPass_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/LightingPass_frag.cso LightingPass_frag.hlsl

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b