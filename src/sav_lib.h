#ifndef SAV_LIB_H
#define SAV_LIB_H

#include <varand/varand_types.h>

#include <sdl2/SDL.h>

#include <windows.h>

#ifdef SAV_EXPORTS
#define SAV_API extern "C" __declspec(dllexport)
#else
#define SAV_API extern "C" __declspec(dllimport)
#endif

#define GAME_API extern "C" __declspec(dllexport)

typedef void (*RenderDecl)(b32 *quit, b32 *isInitialized, SDL_Window *window, int currentFrame, u32 *shaderProgram, u32 *vao, u32 *vbo);

struct game_code
{
    b32 isValid;
    HMODULE dll;
    FILETIME lastWriteTime;

    RenderDecl Render;
};

struct music_stream
{
    void *music;
};

struct sound_chunk
{
    void *sound;
};

SAV_API b32 InitWindow(SDL_Window **window, const char *name, int width, int height);
SAV_API void PollEvents(b32 *quit);
SAV_API const u8 *GetSdlKeyboardState();
SAV_API void Quit(SDL_Window *window);
SAV_API void Win32LoadGameCode(game_code *gameCode, const char *sourceDllPath, const char *tempDllPath);
SAV_API void Win32UnloadGameCode(game_code *gameCode);
SAV_API void Win32ReloadGameCode(game_code *gameCode, const char *sourceDllPath, const char *tempDllPath, const char *lockFilePath);
SAV_API u32 BuildShader();
SAV_API void PrepareGpuData(u32 *vbo, u32 *vao);
SAV_API void BeginDraw();
SAV_API void DrawVertices(u32 shaderProgram, u32 vbo, u32 vao, float *vertices, int vertexCount);
SAV_API void SetWindowTitle(SDL_Window *window, const char *title);
SAV_API void EndDraw(SDL_Window *window);
SAV_API b32 InitAudio();
SAV_API music_stream LoadMusicStream(const char *filePath);
SAV_API sound_chunk LoadSoundChunk(const char *filePath);
SAV_API b32 PlayMusicStream(music_stream stream);
SAV_API b32 PlaySoundChunk(sound_chunk chunk);
SAV_API void FreeMusicStream(music_stream stream);
SAV_API void FreeSoundChunk(sound_chunk chunk);

#endif
