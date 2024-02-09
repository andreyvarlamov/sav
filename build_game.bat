@echo off

setlocal

set libsdir=C:\dev\shared\libs
set incdir=C:\dev\shared\include

set rootdir=%~dp0
set bindir=%rootdir%bin
set srcdir=%rootdir%src

set copt=/IC:\dev\shared\include /MDd /LDd /Z7 /Od /Oi /FC /GR- /nologo
set cwopt=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505
set llib=sav_lib.lib
set lopt=/libpath:%libsdir% /debug /opt:ref /incremental:no

pushd %bindir%

copy nul savt_game.lock

rem Delete all pdbs if can (can't while still running - visual studio locks them all). And quiet the del "access is denied" output
del savt_game_*.pdb > nul 2> nul

cl %srcdir%\savt_game.cpp %copt% %cwopt% /link %llib% %lopt% /pdb:savt_game_%random%.pdb

del savt_game.lock

popd
