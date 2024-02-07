@echo off

setlocal

set prgname=sav

set libsdir=C:\dev\shared\libs
set incdir=C:\dev\shared\include

set rootdir=%~dp0
set bindir=%rootdir%bin
set srcdir=%rootdir%src

rem Compiler Options
    rem /MTd
    rem /nologo
    rem /FC
    rem /GR-
    rem /Z7
    rem /EHa-
    rem /Od
    rem /Oi
set copt=/I%incdir% /MDd /nologo /FC /GR- /Z7 /EHa- /Od /Oi

rem Compiler Warning Options
    rem /WX
    rem /W4
    rem /wd4201
    rem /wd4100
    rem /wd4189
    rem /wd4505
    rem /wd4996 -- using deprecated functions like fopen, not fopen_s
    rem /wd4456 -- hiding previous declaration (to compile with raygui.h)
    rem /wd4267 -- conversion with possible loss of data - from size_t
set cwopt=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4996 /wd4556 /wd4267

rem Linker options
    rem /libpath
    rem /incremental:no
    rem /opt:ref
    rem /subsystem:console
    rem /verbose
set lopt=/libpath:%libsdir% /incremental:no /opt:ref /subsystem:console

rem Link against libs:
    rem SDL simple -- SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib shell32.lib
    rem SDL opengl assimp -- SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib glad.lib opengl32.lib shell32.lib assimp-vc143-mtd.lib
    rem Raylib -- raylibdll.lib raylib.lib
set llibs=sav-second-dll.lib SDL2main.lib SDL2.lib glad-dll.lib opengl32.lib shell32.lib

pushd %bindir%

cl %srcdir%\%prgname%.cpp^
   %copt% %cwopt% /link %lopt% %llibs%

popd

if %errorlevel% neq 0 (exit /b %errorlevel%)
