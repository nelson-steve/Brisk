@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile Deferred renderer shaders
echo Compiling BasicShader_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/DepthPrePass_vert.cso -Zpr DepthPrePass_vert.hlsl

echo Compiling BasicShader_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/DepthPrePass_frag.cso -Zpr DepthPrePass_frag.hlsl

echo Compiling GeometryPass_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/GeometryPass_vert.cso -Zpr GeometryPass_vert.hlsl

echo Compiling GeometryPass_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/GeometryPass_frag.cso -Zpr GeometryPass_frag.hlsl

echo Compiling LightingPass_vert...
dxc.exe -T vs_6_0 -E VSMain -Fo Compiled/LightingPass_vert.cso -Zpr LightingPass_vert.hlsl

echo Compiling GeometryPass_frag...
dxc.exe -T ps_6_0 -E PSMain -Fo Compiled/LightingPass_frag.cso -Zpr LightingPass_frag.hlsl

echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b