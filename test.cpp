#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "SOIL/src/SOIL.h"
#include "GLM/glm/glm.hpp"
#include "GLM/glm/gtc/matrix_transform.hpp"
#include "GLM/glm/gtc/type_ptr.hpp"
#include <chrono>
#include <vector>
#include <cmath>

const GLchar* vertexSource = R"glsl(
    #version 150 core
    in vec3 position;
    in vec2 texcoord;

    out vec2 Texcoord;
    out vec3 Normal;

    uniform mat4 transl;
    uniform mat4 rotate;
    uniform mat4 view;
    uniform mat4 proj;

    void main()
    {
        Texcoord = texcoord;
        Normal = vec3(rotate * vec4(position, 1.0));
        gl_Position = proj * view * rotate * transl * vec4(position, 1.0);
    }
)glsl";

const GLchar* sunVSource = R"glsl(
    #version 150 core
    in vec3 position;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 proj;

    void main()
    {
        gl_Position = proj * view * model * vec4(position, 1.0);
    }
)glsl";

const GLchar* fragmentSource = R"glsl(
    #version 150 core
    in vec2 Texcoord;
    in vec3 Normal;

    out vec4 outColor;

    uniform sampler2D currTexture;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPos;

    void main()
    {
        vec4 texture = texture(currTexture, Texcoord);
        float ambientStrength = 0.01;
        vec3 ambient = ambientStrength * lightColor;

        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        float specularStrength = 1.0;
        vec3 viewDir = normalize(viewPos - Normal);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
        vec3 specular = specularStrength * spec * lightColor;

        vec4 result = vec4(ambient + diffuse + specular, 1.0);
        outColor = texture * result;
    }
)glsl";

const GLchar* sunFSource = R"glsl(
    #version 150 core
    out vec4 FragColor;

    uniform vec3 lightColor;
    
    void main()
    {
        FragColor = vec4(lightColor, 1.0); // set all 4 vector values to 1.0
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

void makeTexture(const char* filename, const char* shadername, GLenum id, size_t i, GLuint *textures, GLuint shaderProgram) {
    glActiveTexture(id);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    int width, height;
    unsigned char* image = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glUniform1i(glGetUniformLocation(shaderProgram, shadername), (int) i);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void addVertices(float d, float u, float v, std::vector<float> &vertices, float pi) {
    vertices.push_back(d * cos(u)*sin(v)); // x
    vertices.push_back(d * sin(u)*sin(v)); // y
    vertices.push_back(d * cos(v)); // z
    vertices.push_back(u / (2 * pi)); // tex x
    vertices.push_back(v / pi); // tex y
    // vertices.push_back(d * cos(u)*sin(v)); // norm x
    // vertices.push_back(d * sin(u)*sin(v)); // norm y
    // vertices.push_back(d * cos(v)); // norm z
}

std::vector<float> makeGlobe(float d, int res) {
    std::vector<float> vertices;
    float pi = 3.1415926;
    float safe = 0.00001;
    vertices.reserve(6 * 5 * 2 * res * res);
    for (float u = 0.0; u < (2 * pi) - safe; u += pi / res) {
        for (float v = 0.0; v < pi - safe; v += pi / res) {
            addVertices(d, u, v, vertices, pi);
            addVertices(d, u + pi / res, v, vertices, pi);
            addVertices(d, u, v + pi / res, vertices, pi);
            addVertices(d, u + pi / res, v, vertices, pi);
            addVertices(d, u, v + pi / res, vertices, pi);
            addVertices(d, u + pi / res, v + pi / res, vertices, pi);
        }
    }
    return vertices;
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

    GLuint eartha;
    glGenVertexArrays(1, &eartha);
    glBindVertexArray(eartha);
    GLuint moona;
    glGenVertexArrays(1, &moona);
    glBindVertexArray(moona);
    GLuint suna;
    glGenVertexArrays(1, &suna);
    glBindVertexArray(suna);

    GLuint earthb;
    glGenBuffers(1, &earthb);
    std::vector<float> earthbuf = makeGlobe(5.0, 12);
    GLuint moonb;
    glGenBuffers(1, &moonb);
    std::vector<float> moonbuf = makeGlobe(2.5, 24);
    GLuint sunb;
    glGenBuffers(1, &sunb);
    std::vector<float> sunbuf = makeGlobe(6.0, 12);

    glBindBuffer(GL_ARRAY_BUFFER, earthb);
    glBufferData(GL_ARRAY_BUFFER, earthbuf.size() * 4, &earthbuf[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, moonb);
    glBufferData(GL_ARRAY_BUFFER, moonbuf.size() * 4, &moonbuf[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, sunb);
    glBufferData(GL_ARRAY_BUFFER, sunbuf.size() * 4, &sunbuf[0], GL_STATIC_DRAW);

    GLuint vertexShader = makeShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint sunVShader = makeShader(GL_VERTEX_SHADER, sunVSource);
    GLuint sunFShader = makeShader(GL_FRAGMENT_SHADER, sunFSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLuint sunProgram = glCreateProgram();
    glAttachShader(sunProgram, sunVShader);
    glAttachShader(sunProgram, sunFShader);
    glLinkProgram(sunProgram);

    glBindVertexArray(eartha); // to do: merge these three into one
    glBindBuffer(GL_ARRAY_BUFFER, earthb);
    glEnableVertexAttribArray(0);
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (char*)(3 * sizeof(float)));

    glBindVertexArray(moona);
    glBindBuffer(GL_ARRAY_BUFFER, moonb);
    glEnableVertexAttribArray(0);
    posAttrib = glGetAttribLocation(shaderProgram, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (char*)(3 * sizeof(float)));

    glUseProgram(sunProgram);
    glBindVertexArray(suna);
    glBindBuffer(GL_ARRAY_BUFFER, sunb);
    glEnableVertexAttribArray(0);
    posAttrib = glGetAttribLocation(sunProgram, "position");
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

    glUseProgram(shaderProgram);
    GLuint textures[2];
    glGenTextures(2, textures);
    makeTexture("earth.jpg", "texEarth", GL_TEXTURE0, 0, textures, shaderProgram);
    makeTexture("moon.jpg", "texMoon", GL_TEXTURE1, 1, textures, shaderProgram);

    glm::vec3 lightColor(0.95f, 1.0f, 0.81f);
    glm::vec3 lightPos(20.0f, 800.0f, 1.0f);
    glm::vec3 viewPos(50.0f, 50.1f, 7.4f);

    glm::mat4 earthmodel = glm::mat4(1.0f);
    glm::mat4 moonmodel = glm::mat4(1.0f);
    glm::mat4 earthposm = glm::mat4(1.0f);
    glm::mat4 moonposm = glm::mat4(1.0f);
    glm::mat4 sunmodel = glm::mat4(1.0f);
    moonposm = glm::translate(
        moonposm,
        glm::vec3(36.0f, 0.0f, 0.0f)
    );
    sunmodel = glm::translate(
        moonmodel,
        lightPos
    );

    glm::mat4 view = glm::lookAt(
        viewPos,
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    glm::mat4 proj = glm::perspective(glm::radians(55.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

    bool rotate = false;
    auto t_start = std::chrono::high_resolution_clock::now();
    glEnable(GL_DEPTH_TEST);
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    GLint uniLight = glGetUniformLocation(shaderProgram, "lightColor");
    glUniform3fv(uniLight, 1, glm::value_ptr(lightColor));
    GLint uniPos = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3fv(uniPos, 1, glm::value_ptr(lightPos));
    GLint uniVos = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3fv(uniVos, 1, glm::value_ptr(viewPos));

    while (true) {
        if (SDL_PollEvent(&windowEvent)) {
            if (windowEvent.type == SDL_QUIT) break;
            if (windowEvent.type == SDL_KEYUP) {
                if (windowEvent.key.keysym.sym == SDLK_ESCAPE) break;
                if (windowEvent.key.keysym.sym == SDLK_SPACE) rotate = !rotate;
            }
        }

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        if (rotate && time > 0.02f) {
            t_start = t_now;
            earthmodel = glm::rotate(
                earthmodel,
                glm::radians(1.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
            moonmodel = glm::rotate(
                moonmodel,
                glm::radians(-0.4f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
        }

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(shaderProgram, "currTexture"), 0);
        glBindVertexArray(eartha);
        GLint uniModel = glGetUniformLocation(shaderProgram, "transl");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(earthposm));
        uniModel = glGetUniformLocation(shaderProgram, "rotate");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(earthmodel));
        glDrawArrays(GL_TRIANGLES, 0, earthbuf.size());

        glActiveTexture(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(shaderProgram, "currTexture"), 1);
        glBindVertexArray(moona);
        uniModel = glGetUniformLocation(shaderProgram, "transl");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(moonposm));
        uniModel = glGetUniformLocation(shaderProgram, "rotate");
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(moonmodel));
        glDrawArrays(GL_TRIANGLES, 0, moonbuf.size());

        glUseProgram(sunProgram);
        glBindVertexArray(suna);
        GLint sunModel = glGetUniformLocation(sunProgram, "model");
        glUniformMatrix4fv(sunModel, 1, GL_FALSE, glm::value_ptr(sunmodel));
        GLint sunView = glGetUniformLocation(sunProgram, "view");
        glUniformMatrix4fv(sunView, 1, GL_FALSE, glm::value_ptr(view));
        GLint sunProj = glGetUniformLocation(sunProgram, "proj");
        glUniformMatrix4fv(sunProj, 1, GL_FALSE, glm::value_ptr(proj));
        GLint sunLight = glGetUniformLocation(sunProgram, "lightColor");
        glUniform3fv(sunLight, 1, glm::value_ptr(lightColor));
        glDrawArrays(GL_TRIANGLES, 0, sunbuf.size());

        SDL_GL_SwapWindow(window);
    }

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteBuffers(1, &sunb);
    glDeleteBuffers(1, &moonb);
    glDeleteBuffers(1, &earthb);
    glDeleteVertexArrays(1, &suna);
    glDeleteVertexArrays(1, &moona);
    glDeleteVertexArrays(1, &eartha);

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
