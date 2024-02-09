#ifndef SAV_LIB_H
#define SAV_LIB_H

#include <varand/varand_types.h>

#ifdef SAV_EXPORTS
#define SAV_API extern "C" __declspec(dllexport)
#else
#define SAV_API extern "C" __declspec(dllimport)
#endif

#define GAME_API extern "C" __declspec(dllexport)

struct game_memory
{
    void *data;
    size_t size;
};

struct music_stream
{
    void *music;
};

struct sound_chunk
{
    void *sound;
};

SAV_API game_memory AllocGameMemory(size_t size);
SAV_API b32 InitWindow(const char *name, int width, int height);
SAV_API void PollEvents(b32 *quit);
SAV_API const u8 *GetSdlKeyboardState();
SAV_API void Quit();
SAV_API void *InitGameCode(const char *dllPath);
SAV_API void *ReloadGameCode();
SAV_API u32 BuildShader();
SAV_API void PrepareGpuData(u32 *vbo, u32 *vao);
SAV_API void BeginDraw();
SAV_API void DrawVertices(u32 shaderProgram, u32 vbo, u32 vao, float *vertices, int vertexCount);
SAV_API void SetWindowTitle(const char *title);
SAV_API void EndDraw();
SAV_API b32 InitAudio();
SAV_API music_stream LoadMusicStream(const char *filePath);
SAV_API sound_chunk LoadSoundChunk(const char *filePath);
SAV_API b32 PlayMusicStream(music_stream stream);
SAV_API b32 PlaySoundChunk(sound_chunk chunk);
SAV_API void FreeMusicStream(music_stream stream);
SAV_API void FreeSoundChunk(sound_chunk chunk);

#endif
