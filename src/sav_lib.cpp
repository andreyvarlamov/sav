#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_sstring.h>

#include <windows.h>

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

#include <sdl2/SDL.h>
#include <sdl2/SDL_mixer.h>

#include <cstdio>

void *Win32AllocMemory(size_t size)
{
    void *memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (memory == NULL)
    {
        printf("Could not virtual alloc.\n");
    }
    
    return memory;
}

game_memory AllocGameMemory(size_t size)
{
    game_memory gameMemory;
    gameMemory.size = Megabytes(128);
    gameMemory.data = Win32AllocMemory(gameMemory.size);
    return gameMemory;
}

struct sdl_state
{
    SDL_Window *window;
    int windowWidth;
    int windowHeight;
};

b32 InitWindow(sdl_state *sdlState, const char *windowName, int windowWidth, int windowHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        sdlState->windowWidth = windowWidth;
        sdlState->windowHeight = windowHeight;
        sdlState->window = SDL_CreateWindow(windowName,
                                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                            sdlState->windowWidth, sdlState->windowHeight,
                                            SDL_WINDOW_OPENGL);// | SDL_WINDOW_RESIZABLE);

        if (sdlState->window)
        {
            SDL_GLContext glContext = SDL_GL_CreateContext(sdlState->window);

            if (glContext)
            {
                gladLoadGLLoader(SDL_GL_GetProcAddress);
                printf("OpenGL loaded\n");
                printf("Vendor: %s\n", glGetString(GL_VENDOR));
                printf("Renderer: %s\n", glGetString(GL_RENDERER));
                printf("Version: %s\n", glGetString(GL_VERSION));

                int screenWidth, screenHeight;
                SDL_GetWindowSize(sdlState->window, &screenWidth, &screenHeight);
                glViewport(0, 0, screenWidth, screenHeight);
            }
            else
            {
                printf("SDL failed to create GL context\n");
                return false;
            }

        }
        else
        {
            printf("SDL failed to create window\n");
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

static_g sdl_state gSdlState;
b32 InitWindow(const char *name, int width, int height)
{
    return InitWindow(&gSdlState, name, width, height);
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

music_stream LoadMusicStream(const char *filePath)
{
    music_stream result;

    Mix_Music *mixMusic = Mix_LoadMUS(filePath);

    result.music = (void *) mixMusic;
    
    return result;
}

sound_chunk LoadSoundChunk(const char *filePath)
{
    sound_chunk result;

    Mix_Chunk *mixChunk = Mix_LoadWAV(filePath);

    result.sound = (void *) mixChunk;
    
    return result;
}

b32 PlayMusicStream(music_stream stream)
{
    return (Mix_PlayMusic((Mix_Music *) stream.music, 0) == 0);
}

b32 PlaySoundChunk(sound_chunk chunk)
{
    int result = Mix_PlayChannel(2, (Mix_Chunk *) chunk.sound, 0);
    return (result != -1);
}

void FreeMusicStream(music_stream stream)
{
    Mix_FreeMusic((Mix_Music *) stream.music);
}

void FreeSoundChunk(sound_chunk chunk)
{
    Mix_FreeChunk((Mix_Chunk *) chunk.sound);
}

void PollEvents(b32 *quit)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            *quit = true;
        }
    }
}

const u8 *GetSdlKeyboardState()
{
    return SDL_GetKeyboardState(0);
}

void Quit(sdl_state *sdlState)
{
   if (sdlState->window)
    {
        SDL_DestroyWindow(sdlState->window);
    }

    SDL_Quit();
}

void Quit()
{
    Quit(&gSdlState);
}

struct game_code
{
    b32 isValid;
    HMODULE dll;
    FILETIME lastWriteTime;

    simple_string sourceDllPath;
    simple_string tempDllPath;
    simple_string lockFilePath;
    
    void *UpdateAndRenderFunc;
};

static FILETIME Win32GetFileModifiedTime(const char *filePath)
{
    FILETIME lastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filePath, GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

void Win32LoadGameCode(game_code *gameCode)
{
    CopyFile(gameCode->sourceDllPath.D, gameCode->tempDllPath.D, FALSE);
    
    gameCode->dll = LoadLibraryA(gameCode->tempDllPath.D);
    
    if (gameCode->dll)
    {
        gameCode->UpdateAndRenderFunc = GetProcAddress(gameCode->dll, "Render");

        gameCode->isValid = (bool) gameCode->UpdateAndRenderFunc;
    }

    gameCode->lastWriteTime = Win32GetFileModifiedTime(gameCode->sourceDllPath.D);
}

void Win32UnloadGameCode(game_code *gameCode)
{
    if (gameCode->dll)
    {
        FreeLibrary(gameCode->dll);
        gameCode->dll = 0;
    }

    gameCode->isValid = false;
    gameCode->UpdateAndRenderFunc = 0;
}

void Win32ReloadGameCode(game_code *gameCode)
{
    FILETIME dllNewWriteTime = Win32GetFileModifiedTime(gameCode->sourceDllPath.D);
    int compareResult = CompareFileTime(&dllNewWriteTime, &gameCode->lastWriteTime);
    
    if (compareResult == 1)
    {
        DWORD lockFileAttrib = GetFileAttributes(gameCode->lockFilePath.D);
        bool lockFilePresent = (lockFileAttrib != INVALID_FILE_ATTRIBUTES);
        
        SYSTEMTIME dllLastWriteTimeSystem;
        int result = FileTimeToSystemTime(&gameCode->lastWriteTime, &dllLastWriteTimeSystem);
                        
        SYSTEMTIME dllNewWriteTimeSystem;
        result = FileTimeToSystemTime(&dllNewWriteTime, &dllNewWriteTimeSystem);

        printf("Old: %02d:%02d:%02d:%03d | New: %02d:%02d:%02d:%03d. Result: %d. Lock file present: %d\n",
               dllLastWriteTimeSystem.wHour,
               dllLastWriteTimeSystem.wMinute,
               dllLastWriteTimeSystem.wSecond,
               dllLastWriteTimeSystem.wMilliseconds,
               dllNewWriteTimeSystem.wHour,
               dllNewWriteTimeSystem.wMinute,
               dllNewWriteTimeSystem.wSecond,
               dllNewWriteTimeSystem.wMilliseconds,
               compareResult,
               lockFilePresent);
 
        // NOTE: Check lock file is not present (if present - rebuild is not done yet)
        if (!lockFilePresent)
        {
            Win32UnloadGameCode(gameCode);
            Win32LoadGameCode(gameCode);
        }
    }
}

static_g game_code gGameCode;
void *InitGameCode(const char *dllPath)
{
    simple_string dir = GetDirectoryFromPath(dllPath);
    simple_string dllNameNoExt = GetFilenameFromPath(dllPath, false);
    simple_string tempDllName = CatStrings(dllNameNoExt.D, "_temp.dll");
    simple_string lockFileName = CatStrings(dllNameNoExt.D, ".lock");
    gGameCode.sourceDllPath = SimpleString(dllPath);
    gGameCode.tempDllPath = CatStrings(dir.D, tempDllName.D);
    gGameCode.lockFilePath = CatStrings(dir.D, lockFileName.D);
    
    Win32LoadGameCode(&gGameCode);

    if (gGameCode.isValid)
    {
        return gGameCode.UpdateAndRenderFunc;
    }
    else
    {
        return 0;
    }
}

void *ReloadGameCode()
{
    Win32ReloadGameCode(&gGameCode);

    if (gGameCode.isValid)
    {
        return gGameCode.UpdateAndRenderFunc;
    }
    else
    {
        return 0;
    }
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
    // printf("original buffer size: %d\n", bufferSize);
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

void SetWindowTitle(sdl_state *sdlState, const char *title)
{
    SDL_SetWindowTitle(sdlState->window, title);
}

void SetWindowTitle(const char *title)
{
    SetWindowTitle(&gSdlState, title);
}

void EndDraw(sdl_state *sdlState)
{
    SDL_GL_SwapWindow(sdlState->window);
}

void EndDraw()
{
    EndDraw(&gSdlState);
}

#if 0
struct gl_state
{
    f32 *vertices;
    i32 vertexCount;
    i32 maxVertexCount;
    u32 shaderProgram;
    u32 vbo;
    u32 vao;
};

static_g gl_state gGlState;

void AddVertex(gl_state *glState, f32 X, f32 Y, f32 Z)
{
    glState->vertices[glState->vertexCount++] = X;
    glState->vertices[glState->vertexCount++] = Y;
    glState->vertices[glState->vertexCount++] = Z;
}

void RenderGL(gl_state *glState)
{
    glBindBuffer(GL_ARRAY_BUFFER, glState->vbo);
    glBufferData(GL_ARRAY_BUFFER, glState->vertexCount * sizeof(f32), glState->vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(glState->shaderProgram);
    glBindVertexArray(glState->vao);
    glDrawArrays(GL_TRIANGLES, 0, glState->vertices);
    glBindVertexArray(0);
}

void AddVertex(f32 X, f32 Y, f32 Z)
{
    AddVertex(&gGlState, X, Y, Z);
}

void RenderGL()
{
    RenderGL(&gGlState);
}
#endif
