#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdio>

using namespace std;

const char *VERTEX_SHADER_SOURCE = R"shader(
#version 410 core

const vec2 VERTICES[6] = vec2[6](
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
);

void main() {
    gl_Position = vec4(VERTICES[gl_VertexID], 0.0, 1.0);  
}
)shader";
const char *FRAGMENT_SHADER_SOURCE = R"shader(
#version 410 core

out vec4 FragColor;

uniform int frame;
uniform ivec2 resolution;

void main() {
    const float surfaceDepth = -20.0f;
    vec3 blackholePos = vec3(0.3f * vec2(cos(frame / 120.0f), sin(frame / 120.0f)), -10.0f);
    vec3 eye = vec3(0.0f, 0.0f, 10.0f);
    vec2 position = gl_FragCoord.xy / resolution * 2.0f - 1.0f;
    position.x *= resolution.x / float(resolution.y);
    vec3 photonPos = vec3(position, 0.0f);
    vec3 rayDirection = normalize(photonPos - eye);
    vec3 velocity = rayDirection * 1.0f;
    int i = 0;
    while(i < 500 && photonPos.z > surfaceDepth) {
        float R = max(length(photonPos - blackholePos), 0.01f);
        float step = clamp(0.1f, R * 0.1f, 2.0f);
        vec3 acc = (1E-1f / (R * R)) * normalize(blackholePos - photonPos);
        velocity += acc * step;
        photonPos += velocity * step;
        if(length(photonPos - blackholePos) < 0.2f) {
            discard;
        }
        ++i;
    }


    if(photonPos.z > surfaceDepth) {
        discard;
    }
    vec2 index = photonPos.xy - (surfaceDepth - photonPos.z) * velocity.xy;
    int color = int(floor(index.x) + floor(index.y)) % 2;
    if(color == 0) {
        FragColor = vec4(1.0, 0.0, 0.0f, 1.0f);
    } else {
        FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    }
}
)shader";

GLuint shaderProgram;
GLFWwindow *window;
GLint frameLocation;
int frame = 0;

void update() {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  glUniform1i(frameLocation, frame);
  ++frame;
}

void windowSizeCallback(GLFWwindow *window, int width, int height) {
  float xscale, yscale;
  glfwGetWindowContentScale(window, &xscale, &yscale);
  width *= xscale;
  height *= yscale;
  glViewport(0, 0, width, height);
  glUniform2i(glGetUniformLocation(shaderProgram, "resolution"), width, height);
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  window = glfwCreateWindow(800, 600, "GLFW Window", NULL, NULL);
  glfwMakeContextCurrent(window);
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failure initializing GLEW");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  char infoLog[512];
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &VERTEX_SHADER_SOURCE, NULL);
  glCompileShader(vertexShader);
  glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
  printf("%s\n", infoLog);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER_SOURCE, NULL);
  glCompileShader(fragmentShader);
  glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
  printf("%s\n", infoLog);

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
  printf("%s\n", infoLog);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(shaderProgram);

  frameLocation = glGetUniformLocation(shaderProgram, "frame");
  glfwSetWindowSizeCallback(window, windowSizeCallback);
  windowSizeCallback(window, 800, 600);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  while (!glfwWindowShouldClose(window)) {
    update();
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
