#include "sav_lib.h"

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>
#include <sdl2/SDL.h>

#include <cstdio>

#include <windows.h>

b32 InitWindow(SDL_Window **window, const char *name, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        *window = SDL_CreateWindow(name,
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   width, height,
                                   SDL_WINDOW_OPENGL);// | SDL_WINDOW_RESIZABLE);

        if (*window)
        {
            SDL_GLContext glContext = SDL_GL_CreateContext(*window);

            if (glContext)
            {
                gladLoadGLLoader(SDL_GL_GetProcAddress);
                printf("OpenGL loaded\n");
                printf("Vendor: %s\n", glGetString(GL_VENDOR));
                printf("Renderer: %s\n", glGetString(GL_RENDERER));
                printf("Version: %s\n", glGetString(GL_VERSION));

                int screenWidth, screenHeight;
                SDL_GetWindowSize(*window, &screenWidth, &screenHeight);
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

void Quit(SDL_Window *window)
{
    if (window)
    {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
}

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

void Win32LoadGameCode(game_code *gameCode, const char *sourceDllPath, const char *tempDllPath)
{
    CopyFile(sourceDllPath, tempDllPath, FALSE);
    
    gameCode->dll = LoadLibraryA(tempDllPath);
    
    if (gameCode->dll)
    {
        gameCode->Render = (RenderDecl)GetProcAddress(gameCode->dll, "Render");

        gameCode->isValid = (bool) gameCode->Render;
    }

    gameCode->lastWriteTime = Win32GetFileModifiedTime(sourceDllPath);
}

void Win32UnloadGameCode(game_code *gameCode)
{
    if (gameCode->dll)
    {
        FreeLibrary(gameCode->dll);
        gameCode->dll = 0;
    }

    gameCode->isValid = false;
    gameCode->Render = 0;
}

void Win32ReloadGameCode(game_code *gameCode, const char *sourceDllPath, const char *tempDllPath, const char *lockFilePath)
{
    FILETIME dllNewWriteTime = Win32GetFileModifiedTime(sourceDllPath);
    int compareResult = CompareFileTime(&dllNewWriteTime, &gameCode->lastWriteTime);
    
    if (compareResult == 1)
    {
        DWORD lockFileAttrib = GetFileAttributes(lockFilePath);
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
            Win32LoadGameCode(gameCode, sourceDllPath, tempDllPath);
        }
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

void SetWindowTitle(SDL_Window *window, const char *title)
{
    SDL_SetWindowTitle(window, title);
}

void EndDraw(SDL_Window *window)
{
    SDL_GL_SwapWindow(window);
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

static gl_state gGlState;

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

