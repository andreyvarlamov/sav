#ifndef SAV_LIB_H
#define SAV_LIB_H

#include <varand/varand_types.h>
#include "va_linmath.h"
#include "va_rect.h"

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

struct music_stream
{
    void *Music;
};

struct sound_chunk
{
    void *Sound;
};

struct sav_image
{
    void *Data;

    int Width;
    int Height;
    int Pitch;

    void *_dataToFree;
};

struct sav_texture
{
    u32 Glid;
    int Width;
    int Height;
};

enum { CAMERA_MAX_ZOOM_STEPS = 16 };

struct camera_2d
{
    vec2 Target;
    vec2 Offset;
    f32 Rotation;
    f32 Zoom;

    f32 ZoomMin;
    f32 ZoomMax;
    f32 ZoomLog;
    
    f32 ZoomLogSteps[CAMERA_MAX_ZOOM_STEPS];
    int ZoomLogStepsCurrent;
    int ZoomLogStepsCount;
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

SAV_API void BeginDraw();
SAV_API void EndDraw();
SAV_API void PrepareGpuData(u32 *VBO, u32 *VAO, u32 *EBO);
SAV_API void DrawVertices(u32 ShaderProgram, u32 VBO, u32 VAO, u32 EBO,
                          vec3 *Positions, vec2 *TexCoords, vec4 *Colors, u32 *Indices,
                          int VertexCount, int IndexCount);
SAV_API void DrawTexture(sav_texture Texture, rect Dest, rect Source, vec2 Origin, f32 Rotation, vec4 Color);
SAV_API void DrawRect(rect Rect, vec4 Color);

SAV_API void BeginCameraMode(camera_2d *Camera);
SAV_API void EndCameraMode();
SAV_API vec2 CameraWorldToScreen(camera_2d *Camera, vec2 World);
SAV_API vec2 CameraScreenToWorld(camera_2d *Camera, vec2 Screen);
SAV_API vec2 CameraScreenToWorldRel(camera_2d *Camera, vec2 Screen);
SAV_API void CameraIncreaseLogZoom(camera_2d *Camera, f32 Delta);
SAV_API void CameraInitLogZoomSteps(camera_2d *Camera, f32 Min, f32 Max, int StepCount);
SAV_API void CameraIncreaseLogZoomSteps(camera_2d *Camera, int Steps);

SAV_API sav_image SavLoadImage(const char *Path);
SAV_API void SavFreeImage(sav_image *Image);
SAV_API sav_texture SavLoadTexture(const char *Path);
SAV_API sav_texture SavLoadTextureFromImage(sav_image Image);
SAV_API sav_texture SavLoadTextureFromData(void *ImageData, u32 Width, u32 Height);

SAV_API void TraceLog(const char *Format, ...);

#endif
