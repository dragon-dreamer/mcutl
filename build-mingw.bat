@echo off
set thispath=%~dp0
set builddir=%thispath%build
if not exist %builddir% mkdir %builddir%
pushd %builddir%
cmake ../ -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if ERRORLEVEL 0 mingw32-make all test
popd
