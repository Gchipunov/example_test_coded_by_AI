#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath> // Use cmath for math functions

float angle = 0.0f;
GLuint programId;
GLint mvpLocation = 0;
int width = 800;
int height = 600;


// gcc -o grok_basic2 grok_basic2.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++


// Vertex shader with corrected matrix multiplication order
std::string vertexShader = R"(
    #version 140
    in vec3 pos;
    in vec4 attr_color;
    uniform mat4 mvp;
    out vec4 v_attr_color;
    void main() {
        v_attr_color = attr_color;
        gl_Position = mvp * vec4(pos, 1.0); // Correct matrix multiplication order
    }
)";

std::string fragmentShader = R"(
    #version 140
    in vec4 v_attr_color;
    void main() {
        gl_FragColor = v_attr_color;
    }
)";

// Compile shader function (unchanged)
GLuint compileShaders(const std::string& shader, GLenum type) {
    const char* shaderCode = shader.c_str();
    GLuint shaderId = glCreateShader(type);
    if (shaderId == 0) {
        std::cout << "Error creating shader!" << std::endl;
        return 0;
    }
    glShaderSource(shaderId, 1, &shaderCode, nullptr);
    glCompileShader(shaderId);
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char* cMessage = new char[length];
        glGetShaderInfoLog(shaderId, length, &length, cMessage);
        std::cout << "Cannot Compile Shader: " << cMessage << std::endl;
        delete[] cMessage;
        glDeleteShader(shaderId);
        return 0;
    }
    return shaderId;
}

// Link program function (unchanged)
GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId) {
    programId = glCreateProgram();
    if (programId == 0) {
        std::cout << "Error Creating Shader Program" << std::endl;
        return 0;
    }
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glBindAttribLocation(programId, 0, "pos");
    glBindAttribLocation(programId, 1, "attr_color");
    glLinkProgram(programId);
    GLint linkStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        std::cout << "Error Linking program" << std::endl;
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        glDeleteProgram(programId);
        return 0;
    }
    return programId;
}

// Helper function to create perspective projection matrix
void createPerspectiveMatrix(float* matrix, float fov, float aspect, float near, float far) {
    float tanHalfFov = tan(fov / 2.0f);
    matrix[0] = 1.0f / (aspect * tanHalfFov); matrix[1] = 0.0f; matrix[2] = 0.0f; matrix[3] = 0.0f;
    matrix[4] = 0.0f; matrix[5] = 1.0f / tanHalfFov; matrix[6] = 0.0f; matrix[7] = 0.0f;
    matrix[8] = 0.0f; matrix[9] = 0.0f; matrix[10] = -(far + near) / (far - near); matrix[11] = -1.0f;
    matrix[12] = 0.0f; matrix[13] = 0.0f; matrix[14] = -(2.0f * far * near) / (far - near); matrix[15] = 0.0f;
}

int main(int argc, char** argv) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "Basic Shapes Examples", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Compile and link shaders
    GLuint vShaderId = compileShaders(vertexShader, GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader, GL_FRAGMENT_SHADER);
    programId = linkProgram(vShaderId, fShaderId);
    glUseProgram(programId);
    mvpLocation = glGetUniformLocation(programId, "mvp");

    // Optimized vertex data: triangles, cube, and rectangles with consistent planes
    GLfloat vertices[] = {
        // Triangle 1 (top-left, flat in Z=0 plane)
        -0.75f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.25f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.75f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.25f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.75f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.25f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right

        // Triangle 2 (middle, flat in Z=0 plane)
        0.0f,   -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        0.25f,  -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        0.0f,    0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left

        // Cube (all faces flat, centered at (0.625, -0.125, 0.125))
        // Front face (Z=0.0f)
        0.50f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-left
        0.75f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-left
        0.75f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-left
        0.75f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-right
        // Back face (Z=0.25f)
        0.50f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-left
        0.75f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-left
        0.75f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-left
        0.75f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-right
        // Left face (X=0.50f)
        0.50f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.50f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.50f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        0.50f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.50f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        0.50f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        // Right face (X=0.75f)
        0.75f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.75f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.75f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        0.75f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.75f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        0.75f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        // Top face (Y=0.0f)
        0.50f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right
        0.50f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right
        0.50f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        // Bottom face (Y=-0.25f)
        0.50f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        0.50f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        0.50f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right

        // Rectangle group (flat in Z=0.0f plane for consistency)
        // Rectangle 1
        -0.70f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.64f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.70f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.64f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.70f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.64f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 2
        -0.61f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.57f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.61f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.57f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.61f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.57f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 3
        -0.54f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.50f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.54f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.50f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.54f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.50f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 4
        -0.47f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.41f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.47f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.41f, -0.72f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.47f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.41f, -0.53f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
    };
    const int vertexCount = 60; // Total vertices: 9 (triangle 1) + 3 (triangle 2) + 36 (cube) + 24 (rectangles)

    // Create VAO and VBO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable depth testing to ensure correct rendering of 3D objects
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Perspective projection (FOV=45 degrees, near=0.1, far=100.0)
        float proj[16];
        createPerspectiveMatrix(proj, 60.0f * 3.14159f / 180.0f, (float)width / height, 0.1f, 100.0f);

        // Rotation matrix (slight rotation to inspect flatness)
       // angle += 0.01f;
        float c = cosf(angle);
        float s = sinf(angle);
        float rot[16] = {
            c, 0.0f, s, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s, 0.0f, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Translation matrix (closer to objects, Z=-1.0 instead of -2.0)
        float trans[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, -3.0f, // Zoom in by moving camera closer
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Compute MVP matrix: proj * rot * trans
        float temp[16], mvp[16];
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                temp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    temp[i*4+j] += proj[i*4+k] * rot[k*4+j];
            }
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) {
                mvp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    mvp[i*4+j] += temp[i*4+k] * trans[k*4+j];
            }

        // Apply MVP matrix
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);

        // Draw objects
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(programId);
    glfwTerminate();
    return 0;
}