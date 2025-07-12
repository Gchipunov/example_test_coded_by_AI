#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // For sinf, cosf, tanf, sqrtf
#include <string.h> // For memset

//gcc main.c -o ray_intersect.exe -lglfw3 -lglew32 -lopengl32 -lm
//./ray_intersect.exe
// https://github.com/Dahuum/MiniRayX
// https://github.com/42paris/minilibx-linux

// gcc camera-mouse-click-ray-intersection-with-a-triangle_main-bard.c -o camera-mouse-click-ray-intersection-with-a-triangle_main-bard.exe -I"D:/glfw-3.4-cmake/build-glfw3-static-install/include/" -I"D:/games_src/example_test_coded_by_AI/freetype/include/freetype2" -L"D:/glfw-3.4-cmake/build-glfw3-static-install/lib" -L"C:/mingw64/x86_64-w64-mingw32/lib" -L"D:/games_src/example_test_coded_by_AI/freetype/lib" -lopengl32 -lglu32 -lglew32 -lglfw3 -lgdi32 -lfreetype -lws2_32

// --- Simple Custom Math Library (for demonstration) ---
// In a real project, use GLM or your preferred math library.
//
//To implement camera ray intersection with a triangle in C using OpenGL 3.3+, GLFW3, and GLEW, we'll combine several concepts:

//Camera Setup: Standard perspective camera using view and projection matrices.

//Mouse Input: Capturing mouse clicks and converting screen coordinates to normalized device coordinates (NDC).

//Ray Generation: Transforming NDC back into world space to create a ray originating from the camera's near plane and extending into the scene.

//Triangle Definition: A simple triangle in world space.

//Ray-Triangle Intersection Algorithm: Implementing a common algorithm (like the Moller-Trumbore algorithm) to check for intersection.

//Rendering: Drawing the triangle and optionally the ray and intersection point.

//Mathematics for Ray Generation and Intersection:

//Screen to World Ray:

//Normalize Device Coordinates (NDC): x_ndc = (2 * mouse_x / screen_width) - 1, y_ndc = 1 - (2 * mouse_y / screen_height). (Note: Y-axis is often flipped between screen and OpenGL).

//Clip Space: vec4 clipCoords = vec4(x_ndc, y_ndc, -1.0f, 1.0f); (Z = -1 for near plane).

//Eye Space: vec4 eyeCoords = inverse(projection) * clipCoords; (Set eyeCoords.z = -1.0f, eyeCoords.w = 0.0f for direction).

//World Space: vec3 rayDir_world = normalize(inverse(view) * eyeCoords).xyz;

//Ray Origin: rayOrigin_world = cameraPosition;

//Moller-Trumbore Ray-Triangle Intersection:
//Given:

//Ray origin O and direction D

//Triangle vertices V0, V1, V2

//Steps:

//Calculate edge vectors: E1 = V1 - V0, E2 = V2 - V0.

//Calculate P = cross(D, E2).

//Calculate determinant det = dot(E1, P).

//If det is close to zero, ray is parallel to triangle (no intersection or lies in plane).

//Calculate inverse determinant inv_det = 1.0 / det.

//Calculate T = O - V0.

//Calculate u = dot(T, P) * inv_det. If u < 0.0 or u > 1.0, no intersection.

//Calculate Q = cross(T, E1).

//Calculate v = dot(D, Q) * inv_det. If v < 0.0 or u + v > 1.0, no intersection.

//Calculate t = dot(E2, Q) * inv_det. If t < 0.0, intersection is behind ray origin.

//If all checks pass, intersection occurs at O + t * D. u and v are barycentric coordinates.

//This code will be more involved than the previous one due to the math library and event handling. I'll use a basic custom math library for vectors and matrices


typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    float m[16]; // Column-major order
} mat4;

// Vector operations
vec3 vec3_add(vec3 a, vec3 b) { return (vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
vec3 vec3_sub(vec3 a, vec3 b) { return (vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
vec3 vec3_scale(vec3 v, float s) { return (vec3){v.x * s, v.y * s, v.z * s}; }
float vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
float vec3_length(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
vec3 vec3_normalize(vec3 v) {
    float len = vec3_length(v);
    return len > 0 ? vec3_scale(v, 1.0f / len) : (vec3){0,0,0};
}
vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// Matrix operations
void mat4_identity(mat4* out) {
    memset(out->m, 0, sizeof(out->m));
    out->m[0] = 1.0f; out->m[5] = 1.0f; out->m[10] = 1.0f; out->m[15] = 1.0f;
}

void mat4_mul(mat4* out, const mat4* a, const mat4* b) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            out->m[j * 4 + i] =
                a->m[j * 4 + 0] * b->m[0 * 4 + i] +
                a->m[j * 4 + 1] * b->m[1 * 4 + i] +
                a->m[j * 4 + 2] * b->m[2 * 4 + i] +
                a->m[j * 4 + 3] * b->m[3 * 4 + i];
        }
    }
}

vec3 mat4_mul_vec3(const mat4* m, vec3 v, float w) {
    vec3 result;
    result.x = m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z + m->m[12] * w;
    result.y = m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z + m->m[13] * w;
    result.z = m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z + m->m[14] * w;
    return result;
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

void mat4_lookAt(mat4* out, vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 s = vec3_normalize(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    out->m[0] = s.x;  out->m[4] = u.x;  out->m[8]  = -f.x; out->m[12] = 0.0f;
    out->m[1] = s.y;  out->m[5] = u.y;  out->m[9]  = -f.y; out->m[13] = 0.0f;
    out->m[2] = s.z;  out->m[6] = u.z;  out->m[10] = -f.z; out->m[14] = 0.0f;
    out->m[3] = 0.0f; out->m[7] = 0.0f; out->m[11] = 0.0f; out->m[15] = 1.0f;

    mat4 T;
    mat4_identity(&T);
    T.m[12] = -eye.x;
    T.m[13] = -eye.y;
    T.m[14] = -eye.z;

    mat4_mul(out, out, &T);
}


// Simple 4x4 matrix inversion (for view/projection inverse)
// Source: http://stackoverflow.com/questions/11435640/inverse-of-a-4x4-matrix-opengl
void mat4_inverse(mat4* out, const mat4* m) {
    float inv[16], det;
    int i;

    inv[0] = m->m[5] * m->m[10] * m->m[15] -
             m->m[5] * m->m[11] * m->m[14] -
             m->m[9] * m->m[6] * m->m[15] +
             m->m[9] * m->m[7] * m->m[14] +
             m->m[13] * m->m[6] * m->m[11] -
             m->m[13] * m->m[7] * m->m[10];

    inv[4] = -m->m[4] * m->m[10] * m->m[15] +
             m->m[4] * m->m[11] * m->m[14] +
             m->m[8] * m->m[6] * m->m[15] -
             m->m[8] * m->m[7] * m->m[14] -
             m->m[12] * m->m[6] * m->m[11] +
             m->m[12] * m->m[7] * m->m[10];

    inv[8] = m->m[4] * m->m[9] * m->m[15] -
             m->m[4] * m->m[11] * m->m[13] -
             m->m[8] * m->m[5] * m->m[15] +
             m->m[8] * m->m[7] * m->m[13] +
             m->m[12] * m->m[5] * m->m[11] -
             m->m[12] * m->m[7] * m->m[9];

    inv[12] = -m->m[4] * m->m[9] * m->m[14] +
              m->m[4] * m->m[10] * m->m[13] +
              m->m[8] * m->m[5] * m->m[14] -
              m->m[8] * m->m[6] * m->m[13] -
              m->m[12] * m->m[5] * m->m[10] +
              m->m[12] * m->m[6] * m->m[9];

    inv[1] = -m->m[1] * m->m[10] * m->m[15] +
             m->m[1] * m->m[11] * m->m[14] +
             m->m[9] * m->m[2] * m->m[15] -
             m->m[9] * m->m[3] * m->m[14] -
             m->m[13] * m->m[2] * m->m[11] +
             m->m[13] * m->m[3] * m->m[10];

    inv[5] = m->m[0] * m->m[10] * m->m[15] -
             m->m[0] * m->m[11] * m->m[14] -
             m->m[8] * m->m[2] * m->m[15] +
             m->m[8] * m->m[3] * m->m[14] +
             m->m[12] * m->m[2] * m->m[11] -
             m->m[12] * m->m[3] * m->m[10];

    inv[9] = -m->m[0] * m->m[9] * m->m[15] +
             m->m[0] * m->m[11] * m->m[13] +
             m->m[8] * m->m[1] * m->m[15] -
             m->m[8] * m->m[3] * m->m[13] -
             m->m[12] * m->m[1] * m->m[11] +
             m->m[12] * m->m[3] * m->m[9];

    inv[13] = m->m[0] * m->m[9] * m->m[14] -
              m->m[0] * m->m[10] * m->m[13] -
              m->m[8] * m->m[1] * m->m[14] +
              m->m[8] * m->m[2] * m->m[13] +
              m->m[12] * m->m[1] * m->m[10] -
              m->m[12] * m->m[2] * m->m[9];

    inv[2] = m->m[1] * m->m[6] * m->m[15] -
             m->m[1] * m->m[7] * m->m[14] -
             m->m[5] * m->m[2] * m->m[15] +
             m->m[5] * m->m[3] * m->m[14] +
             m->m[13] * m->m[2] * m->m[7] -
             m->m[13] * m->m[3] * m->m[6];

    inv[6] = -m->m[0] * m->m[6] * m->m[15] +
             m->m[0] * m->m[7] * m->m[14] +
             m->m[4] * m->m[2] * m->m[15] -
             m->m[4] * m->m[3] * m->m[14] -
             m->m[12] * m->m[2] * m->m[7] +
             m->m[12] * m->m[3] * m->m[6];

    inv[10] = m->m[0] * m->m[5] * m->m[15] -
              m->m[0] * m->m[7] * m->m[13] -
              m->m[4] * m->m[1] * m->m[15] +
              m->m[4] * m->m[3] * m->m[13] +
              m->m[12] * m->m[1] * m->m[7] -
              m->m[12] * m->m[3] * m->m[5];

    inv[14] = -m->m[0] * m->m[5] * m->m[14] +
              m->m[0] * m->m[6] * m->m[13] +
              m->m[4] * m->m[1] * m->m[14] -
              m->m[4] * m->m[2] * m->m[13] -
              m->m[12] * m->m[1] * m->m[6] +
              m->m[12] * m->m[2] * m->m[5];

    inv[3] = -m->m[1] * m->m[6] * m->m[11] +
             m->m[1] * m->m[7] * m->m[10] +
             m->m[5] * m->m[2] * m->m[11] -
             m->m[5] * m->m[3] * m->m[10] -
             m->m[9] * m->m[2] * m->m[7] +
             m->m[9] * m->m[3] * m->m[6];

    inv[7] = m->m[0] * m->m[6] * m->m[11] -
             m->m[0] * m->m[7] * m->m[10] -
             m->m[4] * m->m[2] * m->m[11] +
             m->m[4] * m->m[3] * m->m[10] +
             m->m[8] * m->m[2] * m->m[7] -
             m->m[8] * m->m[3] * m->m[6];

    inv[11] = -m->m[0] * m->m[5] * m->m[11] +
              m->m[0] * m->m[7] * m->m[9] +
              m->m[4] * m->m[1] * m->m[11] -
              m->m[4] * m->m[3] * m->m[9] -
              m->m[8] * m->m[1] * m->m[7] +
              m->m[8] * m->m[3] * m->m[5];

    inv[15] = m->m[0] * m->m[5] * m->m[10] -
              m->m[0] * m->m[6] * m->m[9] -
              m->m[4] * m->m[1] * m->m[10] +
              m->m[4] * m->m[2] * m->m[9] +
              m->m[8] * m->m[1] * m->m[6] -
              m->m[8] * m->m[2] * m->m[5];

    det = m->m[0] * inv[0] + m->m[1] * inv[4] + m->m[2] * inv[8] + m->m[3] * inv[12];

    if (det == 0) return; // Matrix is singular

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        out->m[i] = inv[i] * det;
}


// --- Global Variables for Camera and Ray/Intersection ---
vec3 cameraPos = {0.0f, 0.0f, 3.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp = {0.0f, 1.0f, 0.0f};

vec3  mouseRayOrigin = {0.0f, 0.0f, 0.0f};
vec3  mouseRayDirection = {0.0f, 0.0f, 0.0f};
GLboolean rayIntersects = GL_FALSE;
vec3 intersectionPoint = {0.0f, 0.0f, 0.0f};

// Triangle vertices (in world space)
vec3 triangleV0 = {-0.5f, -0.5f, 0.0f};
vec3 triangleV1 = { 0.5f, -0.5f, 0.0f};
vec3 triangleV2 = { 0.0f,  0.5f, 0.0f};

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
    "uniform vec4 fragColor;\n" // Uniform for color
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = fragColor;\n"
    "}\0";

// --- Function Prototypes ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
void generateRayFromMouse(GLFWwindow* window, double mouseX, double mouseY,
                          const mat4* projection, const mat4* view,
                          vec3* outRayOrigin, vec3* outRayDirection);
GLboolean rayIntersectsTriangle(vec3 rayOrigin, vec3 rayDirection,
                                vec3 v0, vec3 v1, vec3 v2,
                                vec3* outIntersectionPoint);

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ray-Triangle Intersection", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Required for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Build and compile our shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Set up vertex data for the triangle
    float triangleVertices[] = {
        triangleV0.x, triangleV0.y, triangleV0.z,
        triangleV1.x, triangleV1.y, triangleV1.z,
        triangleV2.x, triangleV2.y, triangleV2.z
    };

    GLuint triangleVBO, triangleVAO;
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set up vertex data for drawing the ray (a line) and intersection point (a point)
    GLuint rayVAO, rayVBO;
    glGenVertexArrays(1, &rayVAO);
    glGenBuffers(1, &rayVBO);

    GLuint pointVAO, pointVBO;
    glGenVertexArrays(1, &pointVAO);
    glGenBuffers(1, &pointVBO);


    // Get uniform locations
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "fragColor");

    glEnable(GL_DEPTH_TEST);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Input
        processInput(window);

        // Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        mat4 model, view, projection;
        mat4_identity(&model); // Triangle is static

        // View matrix
        mat4_lookAt(&view, cameraPos, vec3_add(cameraPos, cameraFront), cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.m);

        // Projection matrix
        mat4_perspective(&projection, (float)M_PI / 4.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.m);

        // Draw the triangle
        glBindVertexArray(triangleVAO);
        glUniform4f(colorLoc, 0.0f, 0.5f, 0.0f, 1.0f); // Green triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw the ray if it was generated by a click
        if (mouseRayDirection.x != 0.0f || mouseRayDirection.y != 0.0f || mouseRayDirection.z != 0.0f) {
            float rayVertices[] = {
                mouseRayOrigin.x, mouseRayOrigin.y, mouseRayOrigin.z, // Ray origin
                mouseRayOrigin.x + mouseRayDirection.x * 10.0f, // Extend ray for visualization
                mouseRayOrigin.y + mouseRayDirection.y * 10.0f,
                mouseRayOrigin.z + mouseRayDirection.z * 10.0f
            };

            glBindVertexArray(rayVAO);
            glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), rayVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow ray
            glLineWidth(2.0f); // Make ray visible
            glDrawArrays(GL_LINES, 0, 2);
            glLineWidth(1.0f); // Reset line width
        }

        // Draw intersection point if it occurred
        if (rayIntersects) {
            float pointVertices[] = {
                intersectionPoint.x, intersectionPoint.y, intersectionPoint.z
            };

            glBindVertexArray(pointVAO);
            glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Red intersection point
            glPointSize(10.0f); // Make point visible
            glDrawArrays(GL_POINTS, 0, 1);
            glPointSize(1.0f); // Reset point size
        }


        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate all resources
    glDeleteVertexArrays(1, &triangleVAO);
    glDeleteBuffers(1, &triangleVBO);
    glDeleteVertexArrays(1, &rayVAO);
    glDeleteBuffers(1, &rayVBO);
    glDeleteVertexArrays(1, &pointVAO);
    glDeleteBuffers(1, &pointVBO);
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
    glViewport(0, 0, width, height);
}

// GLFW: Callback for mouse button events
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        mat4 projection, view;
        mat4_perspective(&projection, (float)M_PI / 4.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mat4_lookAt(&view, cameraPos, vec3_add(cameraPos, cameraFront), cameraUp);

        // Generate the ray
        generateRayFromMouse(window, xpos, ypos, &projection, &view, &mouseRayOrigin, &mouseRayDirection);

        // Check for intersection
        rayIntersects = rayIntersectsTriangle(mouseRayOrigin, mouseRayDirection,
                                              triangleV0, triangleV1, triangleV2,
                                              &intersectionPoint);

        if (rayIntersects) {
            printf("Ray hit triangle at: (%.2f, %.2f, %.2f)\n",
                   intersectionPoint.x, intersectionPoint.y, intersectionPoint.z);
        } else {
            printf("Ray missed triangle.\n");
        }
    }
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

// Generates a ray from mouse coordinates
void generateRayFromMouse(GLFWwindow* window, double mouseX, double mouseY,
                          const mat4* projection, const mat4* view,
                          vec3* outRayOrigin, vec3* outRayDirection) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // 1. Normalize Device Coordinates (NDC)
    float x = (2.0f * (float)mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / height; // Y is inverted in screen space vs NDC

    // 2. Clip Space
    // Near plane point
    vec3 rayClipNear = {x, y, -1.0f}; // Z = -1 for near plane
    // Far plane point (not strictly needed for direction if origin is camera)
    // vec3 rayClipFar = {x, y, 1.0f}; // Z = 1 for far plane

    // 3. Eye Space
    mat4 invProjection;
    mat4_inverse(&invProjection, projection);

    // Convert to eye space (homogeneous coordinates)
    vec3 rayEye = mat4_mul_vec3(&invProjection, rayClipNear, 1.0f);
    rayEye.z = -1.0f; // Pointing into the screen
    float rayEye_w = 0.0f; // For direction vector, w=0

    // 4. World Space
    mat4 invView;
    mat4_inverse(&invView, view);

    // Transform eye space ray to world space
    // For direction vector:
    vec3 rayWorldDir = mat4_mul_vec3(&invView, rayEye, rayEye_w);
    *outRayDirection = vec3_normalize(rayWorldDir);

    // Ray origin is the camera's position in world space
    // You can get this from the inverse of the view matrix's translation component
    // Or, if you keep track of cameraPos, it's simply cameraPos.
    *outRayOrigin = cameraPos;
}

// Moller-Trumbore Ray-Triangle Intersection Algorithm
// Returns GL_TRUE if intersection occurs, GL_FALSE otherwise.
// If intersection occurs, outIntersectionPoint will contain the world coordinates.
GLboolean rayIntersectsTriangle(vec3 rayOrigin, vec3 rayDirection,
                                vec3 v0, vec3 v1, vec3 v2,
                                vec3* outIntersectionPoint) {
    const float EPSILON = 0.0000001f; // A small epsilon value to prevent division by zero

    vec3 edge1 = vec3_sub(v1, v0);
    vec3 edge2 = vec3_sub(v2, v0);

    vec3 pvec = vec3_cross(rayDirection, edge2);
    float det = vec3_dot(edge1, pvec);

    // If det is close to 0, ray is parallel to triangle or lies in the plane
    if (det > -EPSILON && det < EPSILON) {
        return GL_FALSE;
    }

    float invDet = 1.0f / det;
    vec3 tvec = vec3_sub(rayOrigin, v0);
    float u = vec3_dot(tvec, pvec) * invDet;

    if (u < 0.0f || u > 1.0f) {
        return GL_FALSE;
    }

    vec3 qvec = vec3_cross(tvec, edge1);
    float v = vec3_dot(rayDirection, qvec) * invDet;

    if (v < 0.0f || u + v > 1.0f) {
        return GL_FALSE;
    }

    float t = vec3_dot(edge2, qvec) * invDet;

    // Check if intersection point is in front of the ray origin
    if (t > EPSILON) { // Using EPSILON for floating point comparisons
        *outIntersectionPoint = vec3_add(rayOrigin, vec3_scale(rayDirection, t));
        return GL_TRUE;
    }

    // No intersection or intersection behind the ray origin
    return GL_FALSE;
}