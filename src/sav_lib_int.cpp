#ifdef SAV_LIB_INT

#define STRING_BUFFER 1024
#define TIMING_STAT_AVG_COUNT 32

struct sdl_state
{
    SDL_Window *window;

    window_size windowSize;
    
    i32 widthBeforeBorderless;
    i32 heightBeforeBorderless;
    i32 xBeforeBorderless;
    i32 yBeforeBorderless;

    u64 perfCounterFreq;
    u64 lastCounter;
    f64 prevDelta;
    f64 deltaSamples[TIMING_STAT_AVG_COUNT];
    int currentTimingStatSample;
    f64 avgDelta;
};

// TODO: Technically all these could be flags in one u8 array. But I'm not sure if it's slower to do bitwise. Need to profile.
struct input_state
{
    u8 currentKeyStates[SDL_NUM_SCANCODES];
    u8 previousKeyStates[SDL_NUM_SCANCODES];
    u8 repeatKeyStates[SDL_NUM_SCANCODES];

    mouse_pos mousePos;
    mouse_pos mouseRelPos;

    b32 isRelMouse;

    u8 currentMouseButtonStates[SDL_BUTTON_X2 + 1];
    u8 previousMouseButtonStates[SDL_BUTTON_X2 + 1];
    u8 clickMouseButtonStates[SDL_BUTTON_X2 + 1];

    i32 mouseWheel;
};

struct game_code
{
    b32 isValid;
    HMODULE dll;
    FILETIME lastWriteTime;

    simple_string sourceDllPath;
    simple_string tempDllPath;
    simple_string lockFilePath;

    simple_string funcName;
    void *UpdateAndRenderFunc;
};

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
#endif

static_g sdl_state gSdlState;
static_g game_code gGameCode;
static_g input_state gInputState = {};
#if 0
static_g gl_state gGlState;
#endif

static_g u64 gCurrentFrame;

#endif
