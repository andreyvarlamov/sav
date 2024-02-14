#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_sstring.h>

#include <windows.h>

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

#include <sdl2/SDL.h>
#include <sdl2/SDL_mixer.h>

#include <cstdio>

#define SAV_LIB_INT
#include "sav_lib_int.cpp"

//
// NOTE: Internal functions
//

static_i void *Win32AllocMemory(size_t Size)
{
    void *Memory = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (Memory == NULL)
    {
        printf("Could not virtual alloc.\n");
    }
    
    return Memory;
}

static_i FILETIME Win32GetFileModifiedTime(const char *FilePath)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if (GetFileAttributesEx(FilePath, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return LastWriteTime;
}

static_i b32 Win32LoadGameCode(game_code *GameCode)
{
    CopyFile(GameCode->SourceDllPath.D, GameCode->TempDllPath.D, FALSE);
    
    GameCode->Dll = LoadLibraryA(GameCode->TempDllPath.D);
    
    if (GameCode->Dll)
    {
        GameCode->UpdateAndRenderFunc = GetProcAddress(GameCode->Dll, GameCode->FuncName.D);

        GameCode->IsValid = (bool) GameCode->UpdateAndRenderFunc;
        GameCode->LastWriteTime = Win32GetFileModifiedTime(GameCode->SourceDllPath.D);

        return true;
    }

    return false;
}

static_i void Win32UnloadGameCode(game_code *GameCode)
{
    if (GameCode->Dll)
    {
        FreeLibrary(GameCode->Dll);
        GameCode->Dll = 0;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRenderFunc = 0;
}

static_i b32 Win32ReloadGameCode(game_code *GameCode)
{
    FILETIME DllNewWriteTime = Win32GetFileModifiedTime(GameCode->SourceDllPath.D);
    int CompareResult = CompareFileTime(&DllNewWriteTime, &GameCode->LastWriteTime);
    
    if (CompareResult == 1)
    {
        DWORD LockFileAttrib = GetFileAttributes(GameCode->LockFilePath.D);
        bool LockFilePresent = (LockFileAttrib != INVALID_FILE_ATTRIBUTES);
        
        SYSTEMTIME DllLastWriteTimeSystem;
        int Result = FileTimeToSystemTime(&GameCode->LastWriteTime, &DllLastWriteTimeSystem);
                        
        SYSTEMTIME DllNewWriteTimeSystem;
        Result = FileTimeToSystemTime(&DllNewWriteTime, &DllNewWriteTimeSystem);

        printf("Old: %02d:%02d:%02d:%03d | New: %02d:%02d:%02d:%03d. Result: %d. Lock file present: %d\n",
               DllLastWriteTimeSystem.wHour,
               DllLastWriteTimeSystem.wMinute,
               DllLastWriteTimeSystem.wSecond,
               DllLastWriteTimeSystem.wMilliseconds,
               DllNewWriteTimeSystem.wHour,
               DllNewWriteTimeSystem.wMinute,
               DllNewWriteTimeSystem.wSecond,
               DllNewWriteTimeSystem.wMilliseconds,
               CompareResult,
               LockFilePresent);
 
        // NOTE: Check lock file is not present (if present - rebuild is not done yet)
        if (!LockFilePresent)
        {
            Win32UnloadGameCode(GameCode);
            return Win32LoadGameCode(GameCode);
        }
    }

    return false;
}

inline f64 GetAvgDelta(f64 *Samples, int SampleCount)
{
    f64 Accum = 0.0f;
    for (int i = 0; i < SampleCount; i++)
    {
        Accum += Samples[i];
    }
    return Accum / TIMING_STAT_AVG_COUNT;
}

//
// NOTE: External functions
//

void DumpGameMemory(game_memory GameMemory)
{
    win32_state *Win32State = &gWin32State;
    game_code *GameCode = &gGameCode;

    if (Win32State->DumpMemoryBlock == NULL)
    {
        simple_string Dir = GetDirectoryFromPath(GameCode->SourceDllPath.D);
        simple_string DllNameNoExt = GetFilenameFromPath(GameCode->SourceDllPath.D, false);
        simple_string DumpFileName = CatStrings(DllNameNoExt.D, "_mem.savdump");
        simple_string DumpFilePath = CatStrings(Dir.D, DumpFileName.D);

        Win32State->DumpMemoryFileHandle = CreateFileA(DumpFilePath.D, GENERIC_READ|GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    
        LARGE_INTEGER MaxSize;
        MaxSize.QuadPart = GameMemory.Size;
        Win32State->DumpMemoryMap = CreateFileMapping(Win32State->DumpMemoryFileHandle,
                                                      0, PAGE_READWRITE, MaxSize.HighPart, MaxSize.LowPart, 0);
        Win32State->DumpMemoryBlock = MapViewOfFile(Win32State->DumpMemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, GameMemory.Size);
    }
    
    CopyMemory(Win32State->DumpMemoryBlock, GameMemory.Data, GameMemory.Size);
}

void ReloadGameMemoryDump(game_memory GameMemory)
{
    win32_state *Win32State = &gWin32State;
    
    if (Win32State->DumpMemoryBlock)
    {
        CopyMemory(GameMemory.Data, Win32State->DumpMemoryBlock, GameMemory.Size);
    }
}

game_memory AllocGameMemory(size_t Size)
{
    game_memory GameMemory;
    GameMemory.Size = Megabytes(128);
    GameMemory.Data = Win32AllocMemory(GameMemory.Size);
    return GameMemory;
}

b32 InitWindow(const char *WindowName, int WindowWidth, int WindowHeight)
{
    sdl_state *SdlState = &gSdlState;
    
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SdlState->WindowSize.OriginalWidth = SdlState->WindowSize.Width = WindowWidth;
        SdlState->WindowSize.OriginalHeight = SdlState->WindowSize.Height = WindowHeight;
        SdlState->Window = SDL_CreateWindow(WindowName,
                                            3160, 40,
                                            SdlState->WindowSize.Width, SdlState->WindowSize.Height,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        if (SdlState->Window)
        {
            SDL_GLContext GlContext = SDL_GL_CreateContext(SdlState->Window);

            if (GlContext)
            {
                gladLoadGLLoader(SDL_GL_GetProcAddress);
                printf("OpenGL loaded\n");
                printf("Vendor: %s\n", glGetString(GL_VENDOR));
                printf("Renderer: %s\n", glGetString(GL_RENDERER));
                printf("Version: %s\n", glGetString(GL_VERSION));

                int ScreenWidth, ScreenHeight;
                SDL_GetWindowSize(SdlState->Window, &ScreenWidth, &ScreenHeight);
                glViewport(0, 0, ScreenWidth, ScreenHeight);

                SdlState->PerfCounterFreq = SDL_GetPerformanceFrequency();
            }
            else
            {
                printf("SDL failed to create GL context\n");
                return false;
            }

        }
        else
        {
            printf("SDL failed to create Window\n");
            return false;
        }
    }
    else
    {
        printf("SDL failed to init\n");
        return false;
    }

    return true;
}

window_size GetWindowSize()
{
    sdl_state *SdlState = &gSdlState;

    return SdlState->WindowSize;
}

void SetWindowBorderless(b32 Borderless)
{
    sdl_state *SdlState = &gSdlState;

    if (Borderless)
    {
        SdlState->WidthBeforeBorderless = SdlState->WindowSize.Width;
        SdlState->HeightBeforeBorderless = SdlState->WindowSize.Height;

        int X, Y;
        SDL_GetWindowPosition(SdlState->Window, &X, &Y);
        SdlState->XBeforeBorderless = X;
        SdlState->YBeforeBorderless = Y;
    }
    
    SDL_SetWindowBordered(SdlState->Window, (SDL_bool) !Borderless);
    
    if (Borderless)
    {
        SDL_MaximizeWindow(SdlState->Window);
    }
    else
    {
        SDL_RestoreWindow(SdlState->Window);

        if (SdlState->WidthBeforeBorderless != 0)
        {
            SDL_SetWindowSize(SdlState->Window, SdlState->WidthBeforeBorderless, SdlState->HeightBeforeBorderless);
            SDL_SetWindowPosition(SdlState->Window, SdlState->XBeforeBorderless, SdlState->YBeforeBorderless);
            SdlState->WidthBeforeBorderless = 0;
            SdlState->HeightBeforeBorderless = 0;
            SdlState-> XBeforeBorderless = 0;
            SdlState->YBeforeBorderless = 0;
        }
    }
}

b32 InitAudio()
{
    if (SDL_Init(SDL_INIT_AUDIO) == 0)
    {
        if (Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 2048) == 0)
        {
            printf("Audio device loaded\n");
            return true;
        }
        else
        {
            printf("SDL failed to open audio device\n");
            return true;
        }
    }
    else
    {
        printf("SDL failed to init audio subsystem\n");
        return false;
    }
}

music_stream LoadMusicStream(const char *FilePath)
{
    music_stream Result;

    Mix_Music *MixMusic = Mix_LoadMUS(FilePath);

    Result.Music = (void *) MixMusic;
    
    return Result;
}

sound_chunk LoadSoundChunk(const char *FilePath)
{
    sound_chunk Result;

    Mix_Chunk *MixChunk = Mix_LoadWAV(FilePath);

    Result.Sound = (void *) MixChunk;
    
    return Result;
}

b32 PlayMusicStream(music_stream Stream)
{
    return (Mix_PlayMusic((Mix_Music *) Stream.Music, 0) == 0);
}

b32 PlaySoundChunk(sound_chunk Chunk)
{
    int Result = Mix_PlayChannel(2, (Mix_Chunk *) Chunk.Sound, 0);
    return (Result != -1);
}

void FreeMusicStream(music_stream Stream)
{
    Mix_FreeMusic((Mix_Music *) Stream.Music);
}

void FreeSoundChunk(sound_chunk Chunk)
{
    Mix_FreeChunk((Mix_Chunk *) Chunk.Sound);
}

u64 GetCurrentFrame()
{
    return gCurrentFrame;
}

void TraceLog(const char *Format, ...)
{
    char FormatBuf[STRING_BUFFER];
    sprintf_s(FormatBuf, "[F %06zu] %s\n", gCurrentFrame, Format);
    
    va_list VarArgs;
    va_start(VarArgs, Format);
    vprintf_s(FormatBuf, VarArgs);
    va_end(VarArgs);
}

void PollEvents(b32 *Quit)
{
    sdl_state *SdlState = &gSdlState;
    input_state *InputState = &gInputState;

    if (SdlState->LastCounter)
    {
        u64 CurrentCounter = SDL_GetPerformanceCounter();
        u64 CounterElapsed = CurrentCounter - SdlState->LastCounter;
        SdlState->LastCounter = CurrentCounter;
        SdlState->PrevDelta = (f64) CounterElapsed / (f64) SdlState->PerfCounterFreq;

        SdlState->DeltaSamples[SdlState->CurrentTimingStatSample++] = SdlState->PrevDelta;
        if (SdlState->CurrentTimingStatSample >= TIMING_STAT_AVG_COUNT)
        {
            SdlState->AvgDelta = GetAvgDelta(SdlState->DeltaSamples, TIMING_STAT_AVG_COUNT);
            SdlState->CurrentTimingStatSample = 0;
        }
    }
    else
    {
        SdlState->LastCounter = SDL_GetPerformanceCounter();
    }
    
    gCurrentFrame++;

    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        InputState->RepeatKeyStates[i] = 0;

        InputState->PreviousKeyStates[i] = InputState->CurrentKeyStates[i];
    }

    for (int i = 0; i < SDL_BUTTON_X2 + 1; i++)
    {
        InputState->ClickMouseButtonStates[i] = 0;

        InputState->PreviousMouseButtonStates[i] = InputState->CurrentMouseButtonStates[i];
    }

    InputState->MouseWheel = 0;
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
            case SDL_QUIT:
            {
                *Quit = true;
            } break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                InputState->CurrentKeyStates[Event.key.keysym.scancode] = (Event.type == SDL_KEYDOWN);
                InputState->RepeatKeyStates[Event.key.keysym.scancode] = Event.key.repeat;
            } break;
            case SDL_MOUSEMOTION:
            {
                // TraceLog("SDL_MOUSEMOTION: Abs(%d, %d); Rel(%d, %d)", Event.motion.x, Event.motion.y, Event.motion.xrel, Event.motion.yrel);
                // NOTE: It seems it's better to update mouse position every frame
            } break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                // TraceLog("%s: %d, %d, %d", Event.type == SDL_MOUSEBUTTONDOWN ? "SDL_MOUSEBUTTONDOWN" : "SDL_MOUSEBUTTONUP", Event.button.button, Event.button.state, Event.button.clicks);
                InputState->CurrentMouseButtonStates[Event.button.button] = (Event.type == SDL_MOUSEBUTTONDOWN);
                if (Event.type == SDL_MOUSEBUTTONDOWN)
                {
                    InputState->ClickMouseButtonStates[Event.button.button] = Event.button.clicks;
                }
            } break;
            case SDL_MOUSEWHEEL:
            {
                // TODO: Maybe deal with Event.wheel.direction field on other platforms
                // TraceLog("SDL_MOUSE_WHELL: %d, %d; %f, %f", Event.wheel.x, Event.wheel.y, Event.wheel.preciseX, Event.wheel.preciseY);
                InputState->MouseWheel += Event.wheel.y; // NOTE: Add y, because it's likely there were more than one Event between frames
            } break;
            case SDL_WINDOWEVENT:
            {
                if (Event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    SdlState->WindowSize.Width = Event.window.data1;
                    SdlState->WindowSize.Height = Event.window.data2;
                    glViewport(0, 0, SdlState->WindowSize.Width, SdlState->WindowSize.Height);
                    // TODO: Update projection matrices
                    // TraceLog("Window resized: %d x %d\n", SdlState->WindowSize.Width, SdlState->WindowSize.Height);
                }
            } break;
            default: break;
        }
    }

    SDL_GetMouseState(&InputState->MousePos.X, &InputState->MousePos.Y);
    SDL_GetRelativeMouseState(&InputState->MouseRelPos.X, &InputState->MouseRelPos.Y);
    // TraceLog("Updated mouse: Abs(%d, %d); Rel(%d, %d)", InputState->MousePos.X, InputState->MousePos.Y, InputState->MouseRelPos.X, InputState->MouseRelPos.Y);
}

f64 GetDeltaFixed()
{
    // TODO: Fixed framerate game logic
    return 0.16;
}

f64 GetDeltaPrev()
{
    sdl_state *SdlState = &gSdlState;
    
    return SdlState->PrevDelta;
}

f64 GetDeltaAvg()
{
    sdl_state *SdlState = &gSdlState;

    return SdlState->AvgDelta;
}

f64 GetFPSPrev()
{
    sdl_state *SdlState = &gSdlState;

    if (SdlState->PrevDelta > 0.0)
    {
        return 1.0 / SdlState->PrevDelta;
    }
    else
    {
        return 0.0;
    }
}

f64 GetFPSAvg()
{
    sdl_state *SdlState = &gSdlState;

    if (SdlState->AvgDelta > 0.0)
    {
        return 1.0 / SdlState->AvgDelta;
    }
    else
    {
        return 0.0;
    }
}

b32 KeyDown(int Key)
{
    input_state *InputState = &gInputState;

    return (b32) InputState->CurrentKeyStates[Key];
}

b32 KeyPressed(int Key)
{
    input_state *InputState = &gInputState;

    return (b32) (InputState->CurrentKeyStates[Key] && !InputState->PreviousKeyStates[Key]);
}

b32 KeyReleased(int Key)
{
    input_state *InputState = &gInputState;

    return (b32) (!InputState->CurrentKeyStates[Key] && InputState->PreviousKeyStates[Key]);
}

b32 KeyRepeat(int Key)
{
    input_state *InputState = &gInputState;

    return (b32) InputState->RepeatKeyStates[Key];
}

b32 KeyPressedOrRepeat(int Key)
{
    input_state *InputState = &gInputState;

    return (b32) ((InputState->CurrentKeyStates[Key] && !InputState->PreviousKeyStates[Key]) || InputState->RepeatKeyStates[Key]);
}

b32 GetMouseRelativeMode()
{
    return (b32) SDL_GetRelativeMouseMode();
}

void SetMouseRelativeMode(b32 Enabled)
{
    input_state *InputState = &gInputState;
    InputState->IsRelMouse = Enabled;
    SDL_SetRelativeMouseMode((SDL_bool) InputState->IsRelMouse);
}

mouse_pos GetMousePos()
{
    input_state *InputState = &gInputState;

    return InputState->MousePos;
}

mouse_pos GetMouseRelPos()
{
    input_state *InputState = &gInputState;

    return InputState->MouseRelPos;
}

b32 MouseDown(int Button)
{
    input_state *InputState = &gInputState;

    return (b32) InputState->CurrentMouseButtonStates[Button];
}

b32 MousePressed(int Button)
{
    input_state *InputState = &gInputState;

    return (b32) (InputState->CurrentMouseButtonStates[Button] && !InputState->PreviousMouseButtonStates[Button]);
}

b32 MouseReleased(int Button)
{
    input_state *InputState = &gInputState;

    return (b32) (!InputState->CurrentMouseButtonStates[Button] && InputState->PreviousMouseButtonStates[Button]);
}

b32 MouseClicks(int Button, int Clicks)
{
    input_state *InputState = &gInputState;

    return (b32) (InputState->ClickMouseButtonStates[Button] == Clicks);
}

i32 MouseWheel()
{
    input_state *InputState = &gInputState;
    
    return InputState->MouseWheel;
}
 
void Quit(sdl_state *SdlState)
{
   if (SdlState->Window)
    {
        SDL_DestroyWindow(SdlState->Window);
    }

    SDL_Quit();
}

void Quit()
{
    Quit(&gSdlState);
}

b32 InitGameCode(const char *DllPath, const char *FuncName, void **UpdateAndRenderFunc)
{
    game_code *GameCode = &gGameCode;
    
    simple_string Dir = GetDirectoryFromPath(DllPath);
    simple_string DllNameNoExt = GetFilenameFromPath(DllPath, false);
    simple_string TempDllName = CatStrings(DllNameNoExt.D, "_temp.Dll");
    simple_string LockFileName = CatStrings(DllNameNoExt.D, ".lock");
    GameCode->SourceDllPath = SimpleString(DllPath);
    GameCode->TempDllPath = CatStrings(Dir.D, TempDllName.D);
    GameCode->LockFilePath = CatStrings(Dir.D,  LockFileName.D);
    GameCode->FuncName = SimpleString(FuncName);
    
    int Loaded = Win32LoadGameCode(GameCode);

    if (Loaded)
    {
        *UpdateAndRenderFunc = GameCode->UpdateAndRenderFunc;
    }
    else
    {
        *UpdateAndRenderFunc = 0;
    }

    return Loaded;
}

b32 ReloadGameCode(void **UpdateAndRenderFunc)
{
    game_code *GameCode = &gGameCode;

    b32 Reloaded = Win32ReloadGameCode(GameCode);

    if (Reloaded)
    {
        if (GameCode->IsValid)
        {
            *UpdateAndRenderFunc = GameCode->UpdateAndRenderFunc;
        }
        else
        {
            *UpdateAndRenderFunc = 0;
        }
    }

    return Reloaded;
}

u32 BuildShader()
{
    const char *vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Vertex shader compilation error:\n%s\n\n", infoLog);
    }
    
    const char *fragmentShaderSource =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";
     
    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Fragment shader compilation error:\n%s\n\n", infoLog);
    }
    
    u32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Linking error:\n%s\n\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void PrepareGpuData(u32 *vbo, u32 *vao)
{
    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), 0, GL_STATIC_DRAW);
    // GLint bufferSize;
    // glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    // printf("Original buffer size: %d\n", bufferSize);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
                    
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void BeginDraw()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DrawVertices(u32 shaderProgram, u32 vbo, u32 vao, float *vertices, int vertexCount)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * 3 * sizeof(*vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
    glUseProgram(0);
}

void SetWindowTitle(const char *title)
{
    sdl_state *SdlState = &gSdlState;
    
    SDL_SetWindowTitle(SdlState->Window, title);
}

void EndDraw()
{
    sdl_state *SdlState = &gSdlState;
    
    SDL_GL_SwapWindow(SdlState->Window);
}

#if 0
void AddVertex(f32 X, f32 Y, f32 Z)
{
    gl_state *glState = &gGlState;
    
    glState->vertices[glState->vertexCount++] = X;
    glState->vertices[glState->vertexCount++] = Y;
    glState->vertices[glState->vertexCount++] = Z;
}

void RenderGL()
{
    gl_state *glState = &gGlState;
    
    glBindBuffer(GL_ARRAY_BUFFER, glState->vbo);
    glBufferData(GL_ARRAY_BUFFER, glState->vertexCount * sizeof(f32), glState->vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(glState->shaderProgram);
    glBindVertexArray(glState->vao);
    glDrawArrays(GL_TRIANGLES, 0, glState->vertices);
    glBindVertexArray(0);
}
#endif
