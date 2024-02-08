@echo off

setlocal

set libsdir=C:\dev\shared\libs
set incdir=C:\dev\shared\include

set rootdir=%~dp0
set bindir=%rootdir%bin
set srcdir=%rootdir%src

set copt=/IC:\dev\shared\include /MDd /LDd /Z7 /Od /Oi /FC /GR- /nologo
set cwopt=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505
set llib=SDL2_mixer.lib SDL2.lib glad-dll.lib
set lopt=/libpath:%libsdir% /debug /opt:ref /incremental:no

pushd %bindir%

rem /section:.shared,rw 
cl %srcdir%\sav_lib.cpp %copt% %cwopt% /DSAV_EXPORTS /link %llib% %lopt%

popd
