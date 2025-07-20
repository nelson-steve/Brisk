@echo off
cd /d "%~dp0"

setlocal

if not exist "Compiled" mkdir Compiled

:: Compile deferred renderer shaders
echo Compiling DepthPrePass.vert...
glslc DepthPrePass.vert -o Compiled/DepthPrePassVS.spv || goto :error

echo Compiling DepthPrePass.frag...
glslc DepthPrePass.frag -o Compiled/DepthPrePassFS.spv || goto :error


echo Compiling GeometryPass.vert...
glslc GeometryPass.vert -o Compiled/GeometryPassVS.spv || goto :error

echo Compiling GeometryPass.frag...
glslc GeometryPass.frag -o Compiled/GeometryPassFS.spv || goto :error


echo Compiling SSAOPass.vert...
glslc SSAOPass.vert -o Compiled/SSAOPassVS.spv || goto :error

echo Compiling SSAOPass.frag...
glslc SSAOPass.frag -o Compiled/SSAOPassFS.spv || goto :error


echo Compiling ShadowMapPass.vert...
glslc ShadowMapPass.vert -o Compiled/ShadowMapPassVS.spv || goto :error

echo Compiling ShadowMapPass.frag...
glslc ShadowMapPass.frag -o Compiled/ShadowMapPassFS.spv || goto :error


echo Compiling LightingPass.vert...
glslc LightingPass.vert -o Compiled/LightingPassVS.spv || goto :error

echo Compiling LightingPass.frag...
glslc LightingPass.frag -o Compiled/LightingPassFS.spv || goto :error


echo Compiling ForwardPass.vert...
glslc ForwardPass.vert -o Compiled/ForwardPassVS.spv || goto :error

echo Compiling ForwardPass.frag...
glslc ForwardPass.frag -o Compiled/ForwardPassFS.spv || goto :error


echo Compiling BloomPass.vert...
glslc BloomPass.vert -o Compiled/BloomPassVS.spv || goto :error

echo Compiling BloomThresholdPass.frag...
glslc BloomThresholdPass.frag -o Compiled/BloomThresholdPassFS.spv || goto :error

echo Compiling BloomBlurPass.frag...
glslc BloomBlurPass.frag -o Compiled/BloomBlurPassFS.spv || goto :error

echo Compiling BloomCombinePass.frag...
glslc BloomCombinePass.frag -o Compiled/BloomCombinePassFS.spv || goto :error


echo All shaders compiled successfully!
pause
exit /b

:error
echo Failed to compile shader!
pause
exit /b
