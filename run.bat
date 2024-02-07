@echo off

setlocal

set prgname=sav

set rootdir=%~dp0
set bindir=%rootdir%bin

pushd %rootdir%

%bindir%\%prgname%.exe

popd
