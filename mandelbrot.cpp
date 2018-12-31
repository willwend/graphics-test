#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <vector>

const GLchar* vertexSource = R"glsl(
    #version 150 core
    in vec2 position;

    out vec2 Position;

    void main()
    {
        Position = position;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";

const GLchar* fragmentSource = R"glsl(
    #version 150 core
    in vec2 Position;

    out vec4 outColor;

    void main()
    {
        //outColor = vec4(0.5,0.0,0.0,1.0);
        float x = 2 * Position.x;
        float y = 2 * Position.y;
        float r = 0;
        float i = 0;
        for (int j = 0; j < 20; j++) {
            float newx = r*r - i*i + x;
            float newy = 2*r*i + y;
            r = newx;
            i = newy;
            if (r > 2.0 || r < -2.0 || i > 2.0 || i < -2.0) {
                outColor = vec4(1.0, 1.0, 1.0, 1.0);
                return;
            }
        }
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
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

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    std::vector<float> vertices;
    vertices.push_back(-1.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(1.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(1.0f);

    vertices.push_back(1.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * 4, &vertices[0], GL_STATIC_DRAW);

    GLuint vertexShader = makeShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    //GLint uniVos = glGetUniformLocation(shaderProgram, "viewPos");
    //glUniform3fv(uniVos, 1, glm::value_ptr(viewPos));

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) break;
            if (windowEvent.type == SDL_KEYUP) {
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE) break;
            }
        }

        glClearColor(0.0f, 0.2f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

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
