@echo off

setlocal

set libsdir=C:\dev\shared\libs
set incdir=C:\dev\shared\include

set rootdir=%~dp0
set bindir=%rootdir%bin
set srcdir=%rootdir%src

rem Compiler Options
    rem /MDd -- Causes the application to use the multithread-specific and DLL-specific version of the run-time library
    rem /nologo
    rem /FC -- Output full path of files in compiler diagnostics
    rem /GR- -- Disable runtime type information
    rem /Z7 -- Produce full-symbolic debugging information in the .obj files using CodeView format
    rem /Od -- Turn off all optimizations
    rem /Oi -- Generate intrinsic functions
set copt=/I%incdir% /MDd /nologo /FC /GR- /Z7 /Od /Oi

rem Compiler Warning Options
    rem /WX -- treat warnings as errors
    rem /W4 -- level 4 warnings
    rem disable the following warnings:
    rem /wd4201 -- nonstandard extension used : nameless struct/union
    rem /wd4100 -- unreferenced formal parameter
    rem /wd4189 -- unreferenced local variablel
    rem /wd4505 -- unreferenced function
    rem Maybe try not to disable those
    rem /wd4996 -- using deprecated functions like fopen, not fopen_s
    rem /wd4456 -- hiding previous declaration (to compile with raygui.h)
    rem /wd4267 -- conversion with possible loss of data - from size_t
set cwopt=/WX /W4 /wd4201 /wd4100 /wd4189 /wd4505

rem Linker options
    rem /libpath
    rem /debug -- generate final 
    rem /incremental:no
    rem /opt:ref -- linker optimization: remove unreferenced functions and data ("comdats") -- mostly to reduce size of exe due to crt
    rem /subsystem:console
    rem /verbose
set lopt=/libpath:%libsdir% /debug /opt:ref /incremental:no /subsystem:console

rem Link against libs:
    rem SDL simple -- SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib shell32.lib
    rem SDL opengl assimp -- SDL2main.lib SDL2.lib SDL2_image.lib SDL2_ttf.lib glad.lib opengl32.lib shell32.lib assimp-vc143-mtd.lib
    rem Raylib -- raylibdll.lib raylib.lib
set llib=sav_lib.lib SDL2main.lib SDL2.lib glad-dll.lib opengl32.lib shell32.lib

pushd %bindir%

cl %srcdir%\savt_platform.cpp %copt% %cwopt% /link %llib% %lopt%

popd

if %errorlevel% neq 0 (exit /b %errorlevel%)
