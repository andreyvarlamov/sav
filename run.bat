@echo off

setlocal

set rootdir=%~dp0
set bindir=%rootdir%bin

pushd %rootdir%

%bindir%\savt_platform.exe

popd
