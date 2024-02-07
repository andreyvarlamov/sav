#include <varand/varand_types.h>

#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>
#include <sdl2/SDL.h>

#include <cstdio>

#include <vector>

extern "C" int GetSum(int a, int b);
// int GetSum(int a, int b, int c)
// {
//     return GetSum(a, b + c);
// }


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

extern "C" void Render(b32 *quit, b32 *isInitialized, SDL_Window *window, int currentFrame, u32 *shaderProgram, u32 *vbo, u32 *vao) 
{
    if (!*isInitialized)
    {
        printf("%u, %u: %d \n", *shaderProgram, vao, GetSum(10, 10));

        *shaderProgram = BuildShader();

        glGenBuffers(1, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), 0, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, vao);
        glBindVertexArray(*vao);
                    
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        
        *isInitialized = true;
    }

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            *quit = true;
        }
    }

    const u8 *sdlKeyboardState = SDL_GetKeyboardState(0);
    if (sdlKeyboardState[SDL_SCANCODE_ESCAPE])
    {
        *quit = true;
    }

    if (sdlKeyboardState[SDL_SCANCODE_2])
    {
        printf("Main exe: 2 button\n");
    }

    if (currentFrame % 60 == 0)
    {
        printf("%u, %u, %u \n", *vbo, *vao, *shaderProgram);
    }

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    float triangle[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(triangle), triangle);

    glUseProgram(*shaderProgram);
    glBindVertexArray(*vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    char title[256];
    sprintf_s(title, "SAV (%d)", GetSum(100, currentFrame / 100));
    SDL_SetWindowTitle(window, title);

    SDL_GL_SwapWindow(window);
}
