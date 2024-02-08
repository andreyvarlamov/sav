#include "sav_lib.h"

#include <varand/varand_util.h>
#include <varand/varand_types.h>

#include <windows.h>

// #define GLAD_GLAPI_EXPORT
// #include <glad/glad.h>
#include <sdl2/SDL.h>
// IMPORTANT: I THINK this is ok to do on all platorms. On windows for windows subsystem this will need to be WinMain.
// When SDL_Init is called it doesn't return with an error that application didn't initialize properly.
// Because SDL_MainIsReady is always set to true, unless SDL_MAIN_NEEDED is defined.
// It seems that SDL_MAIN_NEEDED is only defined for ngage platform (Nokia????? lmao).
// https://github.com/search?q=repo%3Alibsdl-org%2FSDL%20SDL_MAIN_NEEDED&type=code
// In addition to setting SDL_MainIsReady, it parses argc and argv, but I can do it myself on platform layer if need be.
// Then I don't have to link platform against SDL2main.lib and SDL2.lib. Just sav_lib has to be linked against SDL2.lib and glad.lib!!!
#undef main

#include <cstdio>

int main(int argc, char **argv)
{
    SDL_Window *window = 0;
    
    if (InitWindow(&window, "SAV", 1920, 1080))
    {
        const char *gameCodeDllPath = "C:/dev/sav/bin/savt_game.dll";
        const char *gameCodeTempDllPath = "C:/dev/sav/bin/savt_game_temp.dll";
        const char *lockFilePath = "C:/dev/sav/bin/.lock";

        game_code gameCodeData = {};
        game_code *gameCode = &gameCodeData;
                    
        Win32LoadGameCode(gameCode, gameCodeDllPath, gameCodeTempDllPath);

        int currentFrame = 0;

        u32 shaderProgram = 0;
        u32 vbo = 0;
        u32 vao = 0;

        b32 isInitialized = false;
        b32 quit = false;
        while (!quit)
        {
            PollEvents(&quit);
            
            const u8 *sdlKeyboardState = GetSdlKeyboardState();
            if (sdlKeyboardState[SDL_SCANCODE_ESCAPE])
            {
                quit = true;
            }

            Win32ReloadGameCode(gameCode, gameCodeDllPath, gameCodeTempDllPath, lockFilePath);
                        
            if (gameCode->Render)
            {
                gameCode->Render(&quit, &isInitialized, window, currentFrame, &shaderProgram, &vbo, &vao);
            }

            currentFrame++;
        }
    }

    Quit(window);
    
    return 0;
}
