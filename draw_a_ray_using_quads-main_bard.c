#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For strlen

// draw_a_ray_using_quads-main_bard.c
// gcc draw_a_ray_using_quads-main_bard.c -o draw_a_ray_using_quads-main_bard -I"D:/glfw-3.4-cmake/build-glfw3-static-install/include/" -I"D:/games_src/example_test_coded_by_AI/freetype/include/freetype2" -L"D:/glfw-3.4-cmake/build-glfw3-static-install/lib" -lglfw -lGLEW -lGL -lm

// gcc draw_a_ray_using_quads-main_bard.c -o draw_a_ray_using_quads-main_bard -I"D:/glfw-3.4-cmake/build-glfw3-static-install/include/" -I"D:/games_src/example_test_coded_by_AI/freetype/include/freetype2" -L"D:/glfw-3.4-cmake/build-glfw3-static-install/lib" -L"C:/mingw64/x86_64-w64-mingw32/lib" -L"D:/games_src/example_test_coded_by_AI/freetype/lib" -lopengl32 -lglu32 -lglew32 -lglfw3 -lgdi32 -lfreetype -lws2_32

// https://github.com/abedef/Tinker
// Simple GLM-like math for demonstration (you'd typically use a math library like GLM)

#ifndef M_PI

#define M_PI 3.1459

#endif

typedef struct {
    float m[16];
} mat4;

void mat4_identity(mat4* out) {
    memset(out->m, 0, sizeof(out->m));
    out->m[0] = 1.0f; out->m[5] = 1.0f; out->m[10] = 1.0f; out->m[15] = 1.0f;
}

void mat4_ortho(mat4* out, float left, float right, float bottom, float top, float near, float far) {
    mat4_identity(out);
    out->m[0] = 2.0f / (right - left);
    out->m[5] = 2.0f / (top - bottom);
    out->m[10] = -2.0f / (far - near);
    out->m[12] = -(right + left) / (right - left);
    out->m[13] = -(top + bottom) / (top - bottom);
    out->m[14] = -(far + near) / (far - near);
}

void mat4_perspective(mat4* out, float fov_rad, float aspect, float near, float far) {
    mat4_identity(out);
    float tan_half_fov = tanf(fov_rad / 2.0f);
    out->m[0] = 1.0f / (aspect * tan_half_fov);
    out->m[5] = 1.0f / tan_half_fov;
    out->m[10] = -(far + near) / (far - near);
    out->m[11] = -1.0f;
    out->m[14] = -(2.0f * far * near) / (far - near);
    out->m[15] = 0.0f;
}

void mat4_lookAt(mat4* out, float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ, float upX, float upY, float upZ) {
    float fX = centerX - eyeX;
    float fY = centerY - eyeY;
    float fZ = centerZ - eyeZ;

    float lenF = sqrtf(fX*fX + fY*fY + fZ*fZ);
    fX /= lenF; fY /= lenF; fZ /= lenF;

    float sX = fY * upZ - fZ * upY;
    float sY = fZ * upX - fX * upZ;
    float sZ = fX * upY - fY * upX;

    float lenS = sqrtf(sX*sX + sY*sY + sZ*sZ);
    sX /= lenS; sY /= lenS; sZ /= lenS;

    float uX = sY * fZ - sZ * fY;
    float uY = sZ * fX - sX * fZ;
    float uZ = sX * fY - sY * fX;

    out->m[0] = sX;  out->m[4] = uX;  out->m[8]  = -fX; out->m[12] = 0.0f;
    out->m[1] = sY;  out->m[5] = uY;  out->m[9]  = -fY; out->m[13] = 0.0f;
    out->m[2] = sZ;  out->m[6] = uZ;  out->m[10] = -fZ; out->m[14] = 0.0f;
    out->m[3] = 0.0f; out->m[7] = 0.0f; out->m[11] = 0.0f; out->m[15] = 1.0f;

    float eye_dot_s = eyeX * sX + eyeY * sY + eyeZ * sZ;
    float eye_dot_u = eyeX * uX + eyeY * uY + eyeZ * uZ;
    float eye_dot_f = eyeX * fX + eyeY * fY + eyeZ * fZ;

    out->m[12] = -(eye_dot_s);
    out->m[13] = -(eye_dot_u);
    out->m[14] = (eye_dot_f); // Note: -(-eye_dot_f) due to column-major vs row-major
}


// Shader sources
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color\n"
    "}\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for MacOS
#endif

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Ray with Quads", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Required for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Print OpenGL version
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Build and compile our shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Set up vertex data (and buffer(s)) and configure vertex attributes
    // We'll define a very thin quad to simulate a ray along the X-axis
    // (0,0,0) to (2,0,0) is the "ray" direction and length
    // The quad will extend slightly in the Y-axis to give it thickness
    float vertices[] = {
        // positions         
        // First triangle of the quad
        0.0f,  0.01f, 0.0f,  // Top-left (relative to ray direction)
        2.0f,  0.01f, 0.0f,  // Top-right
        2.0f, -0.01f, 0.0f,  // Bottom-right

        // Second triangle of the quad
        0.0f,  0.01f, 0.0f,  // Top-left
        2.0f, -0.01f, 0.0f,  // Bottom-right
        0.0f, -0.01f, 0.0f   // Bottom-left
    };


    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Get uniform locations
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Input
        processInput(window);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Activate shader
        glUseProgram(shaderProgram);

        // Create transformations
        mat4 model, view, projection;

        // Model matrix (identity for now, ray at origin)
        mat4_identity(&model);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.m);

        // View matrix (camera position and orientation)
        // Camera at (0, 0, 5), looking at (0, 0, 0), up is (0, 1, 0)
        mat4_lookAt(&view, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);

        // Projection matrix (perspective)
        mat4_perspective(&projection, (float)M_PI / 4.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.m);

        // Draw the quad
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vertices for 2 triangles forming a quad

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// GLFW: Whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// Compile a shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
    }
    return shader;
}

// Create a shader program from vertex and fragment shader sources
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}