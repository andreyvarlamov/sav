#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <windows.h>

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>
#include <sdl2/SDL.h>


#include <cstdio>

extern "C" int GetSum(int a, int b);

// extern "C" void Render(b32 *quit, b32 *isInitialized, u32 shaderProgram, u32 vao, SDL_Window *window, int currentFrame);

static bool gameCodeIsValid;
static HMODULE gameCodeDll;
static FILETIME dllLastWriteTime;

typedef void (*RenderDecl)(b32 *quit, b32 *isInitialized, SDL_Window *window, int currentFrame, u32 *shaderProgram, u32 *vao, u32 *vbo);

static RenderDecl Render;

FILETIME Win32GetLastWriteTime(const char *filepath)
{
    FILETIME lastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx(filepath, GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

void Win32LoadGameCode(const char *sourceDllName, const char *tempDllName)
{
    dllLastWriteTime = Win32GetLastWriteTime(sourceDllName);
    
    CopyFile(sourceDllName, tempDllName, FALSE);
    gameCodeDll = LoadLibraryA(tempDllName);
    if (gameCodeDll)
    {
        Render = (RenderDecl)GetProcAddress(gameCodeDll, "Render");

        gameCodeIsValid = Render;
    }
}

void Win32UnloadGameCode()
{
    if (gameCodeDll)
    {
        FreeLibrary(gameCodeDll);
        gameCodeDll = 0;
    }

    gameCodeIsValid = false;
    Render = 0;
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_Window *window = SDL_CreateWindow("SAV",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              1920, 1080,
                                              SDL_WINDOW_OPENGL);// | SDL_WINDOW_RESIZABLE);


        if (window)
        {
            SDL_GLContext glContext = SDL_GL_CreateContext(window);

            if (glContext)
            {
                gladLoadGLLoader(SDL_GL_GetProcAddress);
                printf("OpenGL loaded\n");
                printf("Vendor: %s\n", glGetString(GL_VENDOR));
                printf("Renderer: %s\n", glGetString(GL_RENDERER));
                printf("Version: %s\n", glGetString(GL_VERSION));

                int screenWidth, screenHeight;
                SDL_GetWindowSize(window, &screenWidth, &screenHeight);
                glViewport(0, 0, screenWidth, screenHeight);
                
                // SDL_Surface *windowSurface = SDL_GetWindowSurface(window);

                SDL_Surface *texture = SDL_LoadBMP("res/test.bmp");

                if (texture)
                {
                    const char *gameCodeDllPath = "C:/dev/sav/bin/sav-dll.dll";
                    const char *gameCodeTempDllPath = "C:/dev/sav/bin/sav-dll-temp.dll";
                   
                    Win32LoadGameCode(gameCodeDllPath, gameCodeTempDllPath);

                    int currentFrame = 0;

                    u32 shaderProgram = 0;
                    u32 vbo = 0;
                    u32 vao = 0;

                    b32 isInitialized = false;
                    b32 quit = false;
                    while (!quit)
                    {
                        // printf("From sav: %d\n", GetSum(1, 2));
                        // const u8 *sdlKeyboardState = SDL_GetKeyboardState(0);
                        // if (sdlKeyboardState[SDL_SCANCODE_ESCAPE])
                        // {
                        //     quit = true;
                        // }

                        // if (sdlKeyboardState[SDL_SCANCODE_1])
                        // {
                        //     printf("Main exe: 1 button\n");
                        // }

                        // SDL_BlitSurface(texture, NULL, windowSurface, NULL);

                        // SDL_UpdateWindowSurface(window);


                        FILETIME newDllWriteTime = Win32GetLastWriteTime(gameCodeDllPath);
                        if (CompareFileTime(&newDllWriteTime, &dllLastWriteTime) == 1)
                        {
                            Win32UnloadGameCode();
                            Win32LoadGameCode(gameCodeDllPath, gameCodeTempDllPath);
                        }
                        
                        if (Render)
                        {
                            Render(&quit, &isInitialized, window, currentFrame, &shaderProgram, &vbo, &vao);
                        }

                        // char title[256];
                        // sprintf_s(title, "SAV (%d)", GetSum(100, 100, currentFrame / 100));
                        // SDL_SetWindowTitle(window, title);

                        // SDL_GL_SwapWindow(window);

                        currentFrame++;
                    }
                }
                else
                {
                    // TODO: Logging
                    InvalidCodePath;
                }
            }
            else
            {
                // TODO: Logging
                InvalidCodePath;
            }
        }
        else
        {
            // TODO: Logging
            InvalidCodePath;
        }
        
        SDL_DestroyWindow(window);

        SDL_Quit();
    }
    else
    {
        // TODO: Logging
        InvalidCodePath;
    }
    
    return 0;
}
