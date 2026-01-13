@echo off
cd /d "%~dp0"

setlocal

echo Compiling All shaders...

cd ClusteredLighting
call COMPILE_SHADERS.bat
cd ..

cd DeferredRenderer
call COMPILE_SHADERS.bat
cd ..

cd RayTracing
call COMPILE_SHADERS.bat
cd ..

echo All shaders compiled successfully!
pause