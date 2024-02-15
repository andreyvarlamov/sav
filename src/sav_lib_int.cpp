#ifdef SAV_LIB_INT

#define STRING_BUFFER 1024
#define TIMING_STAT_AVG_COUNT 32

struct sdl_state
{
    SDL_Window *Window;

    window_size WindowSize;
    
    i32 WidthBeforeBorderless;
    i32 HeightBeforeBorderless;
    i32 XBeforeBorderless;
    i32 YBeforeBorderless;

    u64 PerfCounterFreq;
    u64 LastCounter;
    f64 PrevDelta;
    f64 DeltaSamples[TIMING_STAT_AVG_COUNT];
    int CurrentTimingStatSample;
    f64 AvgDelta;
};

struct win32_state
{
    HANDLE DumpMemoryFileHandle;
    HANDLE DumpMemoryMap;
    void *DumpMemoryBlock;
};

// TODO: Technically all these could be flags in one u8 array. But I'm not sure if it's slower to do bitwise. Need to profile.
struct input_state
{
    u8 CurrentKeyStates[SDL_NUM_SCANCODES];
    u8 PreviousKeyStates[SDL_NUM_SCANCODES];
    u8 RepeatKeyStates[SDL_NUM_SCANCODES];

    mouse_pos MousePos;
    mouse_pos MouseRelPos;

    b32 IsRelMouse;

    u8 CurrentMouseButtonStates[SDL_BUTTON_X2 + 1];
    u8 PreviousMouseButtonStates[SDL_BUTTON_X2 + 1];
    u8 ClickMouseButtonStates[SDL_BUTTON_X2 + 1];

    i32 MouseWheel;
};

struct game_code
{
    b32 IsValid;
    HMODULE Dll;
    FILETIME LastWriteTime;

    simple_string SourceDllPath;
    simple_string TempDllPath;
    simple_string LockFilePath;

    simple_string FuncName;
    void *UpdateAndRenderFunc;
};

struct gl_state
{
    u32 ShaderProgram;
    u32 VBO;
    u32 VAO;
    u32 EBO;

    u32 DefaultTextureGlid;
};

static_g sdl_state gSdlState;
static_g win32_state gWin32State;
static_g gl_state gGlState;
static_g input_state gInputState;

static_g game_code gGameCode;

static_g u64 gCurrentFrame;

#endif
