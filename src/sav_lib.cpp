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

static_i void *Win32AllocMemory(size_t size)
{
    void *memory = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (memory == NULL)
    {
        printf("Could not virtual alloc.\n");
    }
    
    return memory;
}

static_i FILETIME Win32GetFileModifiedTime(const char *filePath)
{
    FILETIME lastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filePath, GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

static_i b32 Win32LoadGameCode(game_code *gameCode)
{
    CopyFile(gameCode->sourceDllPath.D, gameCode->tempDllPath.D, FALSE);
    
    gameCode->dll = LoadLibraryA(gameCode->tempDllPath.D);
    
    if (gameCode->dll)
    {
        gameCode->UpdateAndRenderFunc = GetProcAddress(gameCode->dll, "Render");

        gameCode->isValid = (bool) gameCode->UpdateAndRenderFunc;
        gameCode->lastWriteTime = Win32GetFileModifiedTime(gameCode->sourceDllPath.D);

        return true;
    }

    return false;
}

static_i void Win32UnloadGameCode(game_code *gameCode)
{
    if (gameCode->dll)
    {
        FreeLibrary(gameCode->dll);
        gameCode->dll = 0;
    }

    gameCode->isValid = false;
    gameCode->UpdateAndRenderFunc = 0;
}

static_i b32 Win32ReloadGameCode(game_code *gameCode)
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
            return Win32LoadGameCode(gameCode);
        }
    }

    return false;
}

//
// NOTE: External functions
//

game_memory AllocGameMemory(size_t size)
{
    game_memory gameMemory;
    gameMemory.size = Megabytes(128);
    gameMemory.data = Win32AllocMemory(gameMemory.size);
    return gameMemory;
}

b32 InitWindow(const char *windowName, int windowWidth, int windowHeight)
{
    sdl_state *sdlState = &gSdlState;
    
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        sdlState->windowSize.originalWidth = sdlState->windowSize.width = windowWidth;
        sdlState->windowSize.originalHeight = sdlState->windowSize.height = windowHeight;
        sdlState->window = SDL_CreateWindow(windowName,
                                            3160, 40,
                                            sdlState->windowSize.width, sdlState->windowSize.height,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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

window_size GetWindowSize()
{
    sdl_state *sdlState = &gSdlState;

    return sdlState->windowSize;
}

void SetWindowBorderless(b32 borderless)
{
    sdl_state *sdlState = &gSdlState;

    if (borderless)
    {
        sdlState->widthBeforeBorderless = sdlState->windowSize.width;
        sdlState->heightBeforeBorderless = sdlState->windowSize.height;

        int x, y;
        SDL_GetWindowPosition(sdlState->window, &x, &y);
        sdlState->xBeforeBorderless = x;
        sdlState->yBeforeBorderless = y;
    }
    
    SDL_SetWindowBordered(sdlState->window, (SDL_bool) !borderless);
    
    if (borderless)
    {
        SDL_MaximizeWindow(sdlState->window);
    }
    else
    {
        SDL_RestoreWindow(sdlState->window);

        if (sdlState->widthBeforeBorderless != 0)
        {
            SDL_SetWindowSize(sdlState->window, sdlState->widthBeforeBorderless, sdlState->heightBeforeBorderless);
            SDL_SetWindowPosition(sdlState->window, sdlState->xBeforeBorderless, sdlState->yBeforeBorderless);
            sdlState->widthBeforeBorderless = 0;
            sdlState->heightBeforeBorderless = 0;
            sdlState-> xBeforeBorderless = 0;
            sdlState->yBeforeBorderless = 0;
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

u64 GetCurrentFrame()
{
    return gCurrentFrame;
}

void TraceLog(const char *format, ...)
{
    char formatBuf[STRING_BUFFER];
    sprintf_s(formatBuf, "[F %06zu] %s\n", gCurrentFrame, format);
    
    va_list varArgs;
    va_start(varArgs, format);
    vprintf_s(formatBuf, varArgs);
    va_end(varArgs);
}

void PollEvents(b32 *quit)
{
    sdl_state *sdlState = &gSdlState;
    input_state *inputState = &gInputState;
    
    gCurrentFrame++;

    for (int i = 0; i < SDL_NUM_SCANCODES; i++)
    {
        inputState->repeatKeyStates[i] = 0;

        inputState->previousKeyStates[i] = inputState->currentKeyStates[i];
    }

    for (int i = 0; i < SDL_BUTTON_X2 + 1; i++)
    {
        inputState->clickMouseButtonStates[i] = 0;

        inputState->previousMouseButtonStates[i] = inputState->currentMouseButtonStates[i];
    }

    inputState->mouseWheel = 0;
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            {
                *quit = true;
            } break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                inputState->currentKeyStates[event.key.keysym.scancode] = (event.type == SDL_KEYDOWN);
                inputState->repeatKeyStates[event.key.keysym.scancode] = event.key.repeat;
            } break;
            case SDL_MOUSEMOTION:
            {
                // TraceLog("SDL_MOUSEMOTION: Abs(%d, %d); Rel(%d, %d)", event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                // NOTE: It seems it's better to update mouse position every frame
            } break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                // TraceLog("%s: %d, %d, %d", event.type == SDL_MOUSEBUTTONDOWN ? "SDL_MOUSEBUTTONDOWN" : "SDL_MOUSEBUTTONUP", event.button.button, event.button.state, event.button.clicks);
                inputState->currentMouseButtonStates[event.button.button] = (event.type == SDL_MOUSEBUTTONDOWN);
                if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    inputState->clickMouseButtonStates[event.button.button] = event.button.clicks;
                }
            } break;
            case SDL_MOUSEWHEEL:
            {
                // TODO: Maybe deal with event.wheel.direction field on other platforms
                // TraceLog("SDL_MOUSE_WHELL: %d, %d; %f, %f", event.wheel.x, event.wheel.y, event.wheel.preciseX, event.wheel.preciseY);
                inputState->mouseWheel += event.wheel.y; // NOTE: Add y, because it's likely there were more than one event between frames
            } break;
            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    sdlState->windowSize.width = event.window.data1;
                    sdlState->windowSize.height = event.window.data2;
                    glViewport(0, 0, sdlState->windowSize.width, sdlState->windowSize.height);
                    // TODO: Update projection matrices
                    // TraceLog("Window resized: %d x %d\n", sdlState->windowSize.width, sdlState->windowSize.height);
                }
            } break;
            default: break;
        }
    }

    SDL_GetMouseState(&inputState->mousePos.x, &inputState->mousePos.y);
    SDL_GetRelativeMouseState(&inputState->mouseRelPos.x, &inputState->mouseRelPos.y);
    // TraceLog("Updated mouse: Abs(%d, %d); Rel(%d, %d)", inputState->mousePos.x, inputState->mousePos.y, inputState->mouseRelPos.x, inputState->mouseRelPos.y);
}

b32 KeyDown(int key)
{
    input_state *inputState = &gInputState;

    return (b32) inputState->currentKeyStates[key];
}

b32 KeyPressed(int key)
{
    input_state *inputState = &gInputState;

    return (b32) (inputState->currentKeyStates[key] && !inputState->previousKeyStates[key]);
}

b32 KeyReleased(int key)
{
    input_state *inputState = &gInputState;

    return (b32) (!inputState->currentKeyStates[key] && inputState->previousKeyStates[key]);
}

b32 KeyRepeat(int key)
{
    input_state *inputState = &gInputState;

    return (b32) inputState->repeatKeyStates[key];
}

b32 KeyPressedOrRepeat(int key)
{
    input_state *inputState = &gInputState;

    return (b32) ((inputState->currentKeyStates[key] && !inputState->previousKeyStates[key]) || inputState->repeatKeyStates[key]);
}

b32 GetMouseRelativeMode()
{
    return (b32) SDL_GetRelativeMouseMode();
}

void SetMouseRelativeMode(b32 enabled)
{
    input_state *inputState = &gInputState;
    inputState->isRelMouse = enabled;
    SDL_SetRelativeMouseMode((SDL_bool) inputState->isRelMouse);
}

mouse_pos GetMousePos()
{
    input_state *inputState = &gInputState;

    return inputState->mousePos;
}

mouse_pos GetMouseRelPos()
{
    input_state *inputState = &gInputState;

    return inputState->mouseRelPos;
}

b32 MouseDown(int button)
{
    input_state *inputState = &gInputState;

    return (b32) inputState->currentMouseButtonStates[button];
}

b32 MousePressed(int button)
{
    input_state *inputState = &gInputState;

    return (b32) (inputState->currentMouseButtonStates[button] && !inputState->previousMouseButtonStates[button]);
}

b32 MouseReleased(int button)
{
    input_state *inputState = &gInputState;

    return (b32) (!inputState->currentMouseButtonStates[button] && inputState->previousMouseButtonStates[button]);
}

b32 MouseClicks(int button, int clicks)
{
    input_state *inputState = &gInputState;

    return (b32) (inputState->clickMouseButtonStates[button] == clicks);
}

i32 MouseWheel()
{
    input_state *inputState = &gInputState;
    
    return inputState->mouseWheel;
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

b32 InitGameCode(const char *dllPath, void **updateAndRenderFunc)
{
    game_code *gameCode = &gGameCode;
    
    simple_string dir = GetDirectoryFromPath(dllPath);
    simple_string dllNameNoExt = GetFilenameFromPath(dllPath, false);
    simple_string tempDllName = CatStrings(dllNameNoExt.D, "_temp.dll");
    simple_string lockFileName = CatStrings(dllNameNoExt.D, ".lock");
    gameCode->sourceDllPath = SimpleString(dllPath);
    gameCode->tempDllPath = CatStrings(dir.D, tempDllName.D);
    gameCode->lockFilePath = CatStrings(dir.D, lockFileName.D);
    
    int loaded = Win32LoadGameCode(gameCode);

    if (loaded)
    {
        *updateAndRenderFunc = gameCode->UpdateAndRenderFunc;
    }
    else
    {
        *updateAndRenderFunc = 0;
    }

    return loaded;
}

b32 ReloadGameCode(void **updateAndRenderFunc)
{
    game_code *gameCode = &gGameCode;

    b32 reloaded = Win32ReloadGameCode(gameCode);

    if (reloaded)
    {
        if (gameCode->isValid)
        {
            *updateAndRenderFunc = gameCode->UpdateAndRenderFunc;
        }
        else
        {
            *updateAndRenderFunc = 0;
        }
    }

    return reloaded;
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

void SetWindowTitle(const char *title)
{
    sdl_state *sdlState = &gSdlState;
    
    SDL_SetWindowTitle(sdlState->window, title);
}

void EndDraw()
{
    sdl_state *sdlState = &gSdlState;
    
    SDL_GL_SwapWindow(sdlState->window);
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
