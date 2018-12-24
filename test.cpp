#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "SOIL/src/SOIL.h"
#include "GLM/glm/glm.hpp"
#include "GLM/glm/gtc/matrix_transform.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"
#include <chrono>

const GLchar* vertexSource = R"glsl(
    #version 150 core
    in vec2 position;
    in vec3 color;
    in float sides;

    out vec3 vColor;
    out float vSides;

    void main()
    {
        vSides = sides;
        vColor = color;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";

const GLchar* geometrySource = R"glsl(
    #version 150 core
    
    layout(lines) in;
    layout(triangle_strip, max_vertices = 128) out;

    in vec3 vColor[];
    in float vSides[];
    out vec3 fColor;

    const float PI = 3.1415926;
    
    void main()
    {
        for (int j = 0; j < 2; j++) {
            fColor = vColor[j];

            for (int i = 0; i <= vSides[j]; i++) {
                // Angle between each side in radians
                float ang = PI * 2.0 / vSides[j] * i;
        
                // Offset from center of point (0.3 to accomodate for aspect ratio)
                vec4 offset = vec4(cos(ang) * 0.3, -sin(ang) * 0.4, 0.0, 0.0);
                gl_Position = gl_in[j].gl_Position + offset;

                EmitVertex();

                gl_Position = gl_in[j].gl_Position;

                EmitVertex();
            }
        
            EndPrimitive();
        }
    }
)glsl";

const GLchar* fragmentSource = R"glsl(
    #version 150 core
    in vec3 fColor;
    out vec4 outColor;

    void main()
    {
        outColor = vec4(fColor, 1.0);
    }
)glsl";

GLuint makeShader(GLenum type, const GLchar* source) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);
    GLint status;

    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status == GL_TRUE) {
        printf("Shader compiled successfully.\n");
    }
    else {
        char buffer[512];
        glGetShaderInfoLog(id, 512, NULL, buffer);
        printf("Shader failed to compile:\n%s", buffer);
    }
    return id;
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window* window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    glewExperimental = GL_TRUE;
    glewInit();
    SDL_Event windowEvent;

    float vertices[] = {
        -0.45f,  0.45f, 1.0f, 0.0f, 0.0f, 4.0f, // Red point
         0.45f,  0.45f, 0.0f, 1.0f, 0.0f, 8.0f, // Green point
         0.45f, -0.45f, 0.0f, 0.0f, 1.0f, 32.0f, // Blue point
        -0.45f, -0.45f, 1.0f, 1.0f, 0.0f, 6.0f // Yellow point
    };

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    /*GLuint ebo;
    glGenBuffers(1, &ebo); // Create an element array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);*/

    GLuint vertexShader = makeShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint geometryShader = makeShader(GL_GEOMETRY_SHADER, geometrySource);

    GLuint shaderProgram = glCreateProgram(); // combine into program
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    //glBindFragDataLocation(shaderProgram, 0, "outColor"); // which buffer - "glDrawBuffers"
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    GLint sidesAttrib = glGetAttribLocation(shaderProgram, "sides");
    glEnableVertexAttribArray(sidesAttrib);
    glVertexAttribPointer(sidesAttrib, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*) (5 * sizeof(GLfloat)));

    /*GLint texoneAttrib = glGetAttribLocation(shaderProgram, "texcoordone");
    glEnableVertexAttribArray(texoneAttrib);
    glVertexAttribPointer(texoneAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));*/

    /*GLint textwoAttrib = glGetAttribLocation(shaderProgram, "texcoordtwo");
    glEnableVertexAttribArray(textwoAttrib);
    glVertexAttribPointer(textwoAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    // Load texture
    GLuint textures[2];
    glGenTextures(2, textures);*/

    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    int width, height;
    unsigned char* image = SOIL_load_image("SOIL/img_test.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texOne"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/

    /*float pixels[] = {
        0.8f, 0.0f, 0.0f,   0.9f, 0.9f, 0.0f,
        0.0f, 0.6f, 0.9f,   0.0f, 0.8f, 0.0f
    };
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
    glUniform1i(glGetUniformLocation(shaderProgram, "texTwo"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/

    /*glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/
    //GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    //glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

    /*glm::mat4 view = glm::lookAt(
        glm::vec3(2.0f, 3.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(40.0f), 800.0f / 600.0f, 1.0f, 30.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    float sint = 1.6f;
    bool flip = false;
    auto t_start = std::chrono::high_resolution_clock::now();
    glEnable(GL_DEPTH_TEST);
    GLint uniColor = glGetUniformLocation(shaderProgram, "overrideColor");
    glm::mat4 model, old = glm::mat4(1.0f);*/

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) break;
            if (windowEvent.type == SDL_KEYUP) {
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE) break;
                //if (windowEvent.key.keysym.sym == SDLK_SPACE) flip = !flip;
            }
        }

        /*glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Clear the screen to black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniform3f(uniColor, 1.0f, 1.0f, 1.0f); // Start with no darkening

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        sint += time;
        t_start = t_now;
        float scaling = sin(sint) * time + 1.0f;

        model = old;
        if (flip) {
            model = glm::rotate(
                model,
                glm::radians(time * 40.0f),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );
        }
        model = glm::scale(
            model,
            glm::vec3(scaling, scaling, scaling)
        );
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, 36); // Draw cube
        old = model;
        glEnable(GL_STENCIL_TEST); // STENCIL START
        
        // Draw floor
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
        glStencilMask(0xFF);
        glDepthMask(GL_FALSE);
        //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glClear(GL_STENCIL_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 36, 6);

        // Draw cube reflection
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDepthMask(GL_TRUE);
        //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        model = glm::scale(
            glm::translate(model, glm::vec3(0, 0, -1)),
            glm::vec3(1, 1, -1)
        );
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(uniColor, 0.3f, 0.3f, 0.3f); // Darken override
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDisable(GL_STENCIL_TEST); // STENCIL END

        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw rectangle*/

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_LINES, 0, 4);

        SDL_GL_SwapWindow(window);
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
