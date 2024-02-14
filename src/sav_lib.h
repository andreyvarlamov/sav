#ifndef SAV_LIB_H
#define SAV_LIB_H

#include <varand/varand_types.h>
#include <varand/varand_linmath.h>

#ifdef SAV_EXPORTS
#define SAV_API extern "C" __declspec(dllexport)
#else
#define SAV_API extern "C" __declspec(dllimport)
#endif

#define GAME_API extern "C" __declspec(dllexport)

struct game_memory
{
    void *Data;
    size_t Size;
};

struct music_stream
{
    void *Music;
};

struct sound_chunk
{
    void *Sound;
};

struct mouse_pos
{
    int X;
    int Y;
};

struct window_size
{
    int Width;
    int Height;
    int OriginalWidth;
    int OriginalHeight;
};

SAV_API game_memory AllocGameMemory(size_t Size);
SAV_API void DumpGameMemory(game_memory GameMemory);
SAV_API void ReloadGameMemoryDump(game_memory GameMemory);

SAV_API b32 InitGameCode(const char *DllPath, const char *FuncName, void **UpdateAndRenderFunc);
SAV_API b32 ReloadGameCode(void **UpdateAndRenderFunc);

SAV_API b32 InitWindow(const char *Name, int Width, int Height);
SAV_API void PollEvents(b32 *Quit);
SAV_API void Quit();

SAV_API void SetWindowTitle(const char *Title);
SAV_API window_size GetWindowSize();
SAV_API void SetWindowBorderless(b32 Borderless);

SAV_API b32 KeyDown(int Key);
SAV_API b32 KeyPressed(int Key);
SAV_API b32 KeyReleased(int Key);
SAV_API b32 KeyRepeat(int Key);
SAV_API b32 KeyPressedOrRepeat(int Key);
SAV_API b32 GetMouseRelativeMode();
SAV_API void SetMouseRelativeMode(b32 Enabled);
SAV_API mouse_pos GetMousePos();
SAV_API mouse_pos GetMouseRelPos();
SAV_API b32 MouseDown(int Button);
SAV_API b32 MousePressed(int Button);
SAV_API b32 MouseReleased(int Button);
SAV_API b32 MouseClicks(int Button, int Clicks);
SAV_API i32 MouseWheel();

SAV_API u64 GetCurrentFrame();
SAV_API f64 GetDeltaFixed();
SAV_API f64 GetDeltaPrev();
SAV_API f64 GetDeltaAvg();
SAV_API f64 GetFPSPrev();
SAV_API f64 GetFPSAvg();

SAV_API b32 InitAudio();
SAV_API music_stream LoadMusicStream(const char *FilePath);
SAV_API sound_chunk LoadSoundChunk(const char *FilePath);
SAV_API b32 PlayMusicStream(music_stream Stream);
SAV_API b32 PlaySoundChunk(sound_chunk Chunk);
SAV_API void FreeMusicStream(music_stream Stream);
SAV_API void FreeSoundChunk(sound_chunk Chunk);

SAV_API u32 BuildBasicShader(); // TODO: Load shaders from files; // TODO: Shader hot reload
SAV_API void PrepareGpuData(u32 *VBO, u32 *VAO, u32 *EBO);
SAV_API void BeginDraw();
SAV_API void DrawVertices(u32 ShaderProgram, u32 VBO, u32 VAO, u32 EBO,
                          vec3 *Positions, vec2 *TexCoords, vec3 *Colors, u32 *Indices,
                          int VertexCount, int IndexCount);
SAV_API void LoadImage(); // TODO
SAV_API void FreeImage(); // TODO
SAV_API u32 LoadTextureFromData(void *ImageData, u32 Width, u32 Height, u32 Pitch, u32 BytesPerPixel);
SAV_API u32 LoadTexture(const char *Path);
SAV_API void EndDraw();

SAV_API void TraceLog(const char *Format, ...);

#endif
