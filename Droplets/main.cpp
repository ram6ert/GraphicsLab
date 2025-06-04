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

uniform vec2 cursorPos;

float sdf_1(vec2 pos) {
    vec2 size = vec2(200);
    vec2 d = abs(pos - vec2(400)) - size;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
    //return length(pos - vec2(400)) - 200.0f;
}

float sdf_2(vec2 pos) {
    return length(pos - cursorPos) - 200.0f;
}

float smoothUnion(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5 * (d2 - d1) / k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0 - h);
}

void main() {
    float dist1 = sdf_1(gl_FragCoord.xy);
    float dist2 = sdf_2(gl_FragCoord.xy);

    float val = smoothUnion(dist1, dist2, 180);
    if(dist1 < 0) {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if(dist2 < 0) {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else if(val < 0) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        discard;
    }
}
)shader";

GLuint shaderProgram;
GLFWwindow *window;
GLint cursorLocation;

void update() {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  float xscale, yscale;
  glfwGetWindowContentScale(window, &xscale, &yscale);
  float cursor[] = {xscale * (float)x, yscale * (600.0f - (float)y)};
  glUniform2fv(cursorLocation, 1, cursor);
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

  cursorLocation = glGetUniformLocation(shaderProgram, "cursorPos");

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
