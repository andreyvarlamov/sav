#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_sstring.h>
#include <varand/varand_linmath.h>

#include <windows.h>

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

#include <sdl2/SDL.h>
#include <sdl2/SDL_mixer.h>
#include <sdl2/SDL_image.h>

#include <cstdio>

#define SAV_LIB_INT
#include "sav_lib_int.cpp"

//
// ------------------------
// NOTE: Internal functions
// ------------------------
//


static_i void *
Win32AllocMemory(size_t Size)
{
    void *Memory = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (Memory == NULL)
    {
        printf("Could not virtual alloc.\n");
    }
    
    return Memory;
}

static_i FILETIME
Win32GetFileModifiedTime(const char *FilePath)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if (GetFileAttributesEx(FilePath, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return LastWriteTime;
}

static_i b32
Win32LoadGameCode(game_code *GameCode)
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

static_i void
Win32UnloadGameCode(game_code *GameCode)
{
    if (GameCode->Dll)
    {
        FreeLibrary(GameCode->Dll);
        GameCode->Dll = 0;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRenderFunc = 0;
}

static_i b32
Win32ReloadGameCode(game_code *GameCode)
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

inline f64
GetAvgDelta(f64 *Samples, int SampleCount)
{
    f64 Accum = 0.0f;
    for (int i = 0; i < SampleCount; i++)
    {
        Accum += Samples[i];
    }
    return Accum / TIMING_STAT_AVG_COUNT;
}

//
// ------------------------
// NOTE: External functions
// ------------------------
//

//
// NOTE: Game memory
//
game_memory
AllocGameMemory(size_t Size)
{
    game_memory GameMemory;
    GameMemory.Size = Megabytes(128);
    GameMemory.Data = Win32AllocMemory(GameMemory.Size);
    return GameMemory;
}

void
DumpGameMemory(game_memory GameMemory)
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

void
ReloadGameMemoryDump(game_memory GameMemory)
{
    win32_state *Win32State = &gWin32State;
    
    if (Win32State->DumpMemoryBlock)
    {
        CopyMemory(GameMemory.Data, Win32State->DumpMemoryBlock, GameMemory.Size);
    }
}

//
// NOTE: Game code hot reload
//

b32
InitGameCode(const char *DllPath, const char *FuncName, void **UpdateAndRenderFunc)
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

b32
ReloadGameCode(void **UpdateAndRenderFunc)
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

//
// NOTE: Program state/sdl window
//
b32
InitWindow(const char *WindowName, int WindowWidth, int WindowHeight)
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

                i32 SDLImageFlags = IMG_INIT_JPG | IMG_INIT_PNG;
                i32 IMGInitResult = IMG_Init(SDLImageFlags);
                if (!(IMGInitResult & SDLImageFlags))
                {
                    printf("SDL failed to init SDL_image\n");
                    return false;
                }

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

void
PollEvents(b32 *Quit)
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

void
Quit()
{
    sdl_state *SdlState = &gSdlState;
    if (SdlState->Window)
    {
        SDL_DestroyWindow(SdlState->Window);
    }

    SDL_Quit();
}

//
// NOTE: SDL window
//

void
SetWindowTitle(const char *title)
{
    sdl_state *SdlState = &gSdlState;
    
    SDL_SetWindowTitle(SdlState->Window, title);
}

window_size
GetWindowSize()
{
    sdl_state *SdlState = &gSdlState;

    return SdlState->WindowSize;
}

void
SetWindowBorderless(b32 Borderless)
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


//
// NOTE: Input helpers
//

b32 KeyDown(int Key)
{
    return (b32) gInputState.CurrentKeyStates[Key];
}
b32 KeyPressed(int Key)
{
    return (b32) (gInputState.CurrentKeyStates[Key] && !gInputState.PreviousKeyStates[Key]);
}
b32 KeyReleased(int Key)
{
    return (b32) (!gInputState.CurrentKeyStates[Key] && gInputState.PreviousKeyStates[Key]);
}
b32 KeyRepeat(int Key)
{
    return (b32) gInputState.RepeatKeyStates[Key];
}
b32 KeyPressedOrRepeat(int Key)
{
    return (b32) ((gInputState.CurrentKeyStates[Key] && !gInputState.PreviousKeyStates[Key]) || gInputState.RepeatKeyStates[Key]);
}
b32 GetMouseRelativeMode()
{
    return (b32) SDL_GetRelativeMouseMode();
}
void SetMouseRelativeMode(b32 Enabled)
{
    gInputState.IsRelMouse = Enabled;
    SDL_SetRelativeMouseMode((SDL_bool) gInputState.IsRelMouse);
}
mouse_pos GetMousePos()
{
    return gInputState.MousePos;
}
mouse_pos GetMouseRelPos()
{
    return gInputState.MouseRelPos;
}
b32 MouseDown(int Button)
{
    return (b32) gInputState.CurrentMouseButtonStates[Button];
}
b32 MousePressed(int Button)
{
    return (b32) (gInputState.CurrentMouseButtonStates[Button] && !gInputState.PreviousMouseButtonStates[Button]);
}

b32 MouseReleased(int Button)
{
    return (b32) (!gInputState.CurrentMouseButtonStates[Button] && gInputState.PreviousMouseButtonStates[Button]);
}

b32 MouseClicks(int Button, int Clicks)
{
    return (b32) (gInputState.ClickMouseButtonStates[Button] == Clicks);
}
i32 MouseWheel()
{
    return gInputState.MouseWheel;
}

//
// NOTE: Timing
//

u64 GetCurrentFrame()
{
    return gCurrentFrame;
}
f64 GetDeltaFixed()
{
    return 0.16; // TODO: Fixed framerate game logic
}
f64 GetDeltaPrev()
{
    return gSdlState.PrevDelta;
}
f64 GetDeltaAvg()
{
    return gSdlState.AvgDelta;
}
f64 GetFPSPrev()
{
    if (gSdlState.PrevDelta > 0.0) return 1.0 / gSdlState.PrevDelta;
    else return 0.0;
}
f64 GetFPSAvg()
{
    if (gSdlState.AvgDelta > 0.0) return 1.0 / gSdlState.AvgDelta;
    else return 0.0;
}

//
// NOTE: Audio
//

b32
InitAudio()
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

music_stream
LoadMusicStream(const char *FilePath)
{
    music_stream Result;

    Mix_Music *MixMusic = Mix_LoadMUS(FilePath);

    Result.Music = (void *) MixMusic;
    
    return Result;
}

sound_chunk
LoadSoundChunk(const char *FilePath)
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

//
// NOTE: Drawing
//

u32
BuildBasicShader()
{
    const char *vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 vertPosition;\n"
        "layout (location = 1) in vec2 vertTexCoord;\n"
        "layout (location = 2) in vec4 vertColor;\n"
        "out vec2 fragTexCoord;\n"
        "out vec4 fragColor;\n"
        "uniform mat4 mvp;\n"
        "void main()\n"
        "{\n"
        "   fragTexCoord = vertTexCoord;\n"
        "   fragColor = vertColor;\n"
        // "   gl_Position = mvp * vec4(vertPosition, 1.0);\n"
        "   gl_Position = vec4(vertPosition, 1.0);\n"
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
        "in vec2 fragTexCoord;\n"
        "in vec4 fragColor;\n"
        "out vec4 finalColor;\n"
        "uniform sampler2D texture0;\n"
        "uniform vec4 colorDiffuse;\n"
        "void main()\n"
        "{\n"
        "   vec4 texelColor = texture(texture0, fragTexCoord);\n"
        // "   finalColor = texelColor * colorDiffuse * fragColor;\n"
        "   finalColor = texelColor;\n"
        // "   finalColor = vec4(1.0);\n"
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

#define GPU_VERT_COUNT 8192
#define GPU_INDEX_COUNT 32768

void
PrepareGpuData(u32 *VBO, u32 *VAO, u32 *EBO)
{
    u32 MaxVertCount = GPU_VERT_COUNT;
    size_t BytesPerVertex = (3 + 2 + 4) * sizeof(float);
    
    glGenVertexArrays(1, VAO);
    Assert(*VAO);
    glBindVertexArray(*VAO);
                    
    glGenBuffers(1, VBO);
    Assert(*VBO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, MaxVertCount * BytesPerVertex, 0, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) (MaxVertCount * 3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (MaxVertCount * 5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, EBO);
    Assert(*EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, GPU_INDEX_COUNT * sizeof(u32), 0, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void
BeginDraw()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void
DrawVertices(u32 ShaderProgram, u32 VBO, u32 VAO, u32 EBO,
             vec3 *Positions, vec2 *TexCoords, vec3 *Colors, u32 *Indices,
             int VertexCount, int IndexCount)
{
    Assert(Positions);
    Assert(VertexCount > 0);
    
    u32 MaxVertCount = GPU_VERT_COUNT;
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    {
        glBufferSubData(GL_ARRAY_BUFFER, 0,                                VertexCount * 3 * sizeof(float), Positions);
    }
    if (TexCoords)
    {
        glBufferSubData(GL_ARRAY_BUFFER, MaxVertCount * 3 * sizeof(float), VertexCount * 2 * sizeof(float), TexCoords);
    }
    if (Colors)
    {
        glBufferSubData(GL_ARRAY_BUFFER, MaxVertCount * 5 * sizeof(float), VertexCount * 4 * sizeof(float), Colors);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, IndexCount * sizeof(float), Indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glUseProgram(ShaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

u32
LoadTextureFromData(void *ImageData, u32 Width, u32 Height, u32 Pitch, u32 BytesPerPixel)
{
    // TODO: Handle different image data formats better
    Assert(Width * BytesPerPixel == Pitch);
    Assert(BytesPerPixel == 4 || BytesPerPixel == 3);
    
    u32 TextureID;

    glGenTextures(1, &TextureID);
    Assert(TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    u32 InternalFormat = (BytesPerPixel == 4 ? GL_RGBA8 : GL_RGB8);
    u32 Format = (BytesPerPixel == 4 ? GL_RGBA : GL_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0, Format, GL_UNSIGNED_BYTE, ImageData);
    // glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    return TextureID;
}

u32
LoadTexture(const char *Path)
{
    SDL_Surface *surface = IMG_Load(Path);
    // TODO: Handle errors opening images properly
    Assert(surface);

    u32 TextureID = 
        LoadTextureFromData(surface->pixels, (u32) surface->w, (u32) surface->h, (u32) surface->pitch, (u32) surface->format->BytesPerPixel);

    SDL_FreeSurface(surface);

    return TextureID;
}

void
EndDraw()
{
    sdl_state *SdlState = &gSdlState;
    
    SDL_GL_SwapWindow(SdlState->Window);
}

//
// NOTE: Misc
//

void TraceLog(const char *Format, ...)
{
    char FormatBuf[STRING_BUFFER];
    sprintf_s(FormatBuf, "[F %06zu] %s\n", gCurrentFrame, Format);
    
    va_list VarArgs;
    va_start(VarArgs, Format);
    vprintf_s(FormatBuf, VarArgs);
    va_end(VarArgs);
}
