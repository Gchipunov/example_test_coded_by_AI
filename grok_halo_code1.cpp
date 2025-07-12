#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <math.h>

// gcc -o grok_halo_code1 grok_halo_code1.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++


float angle = 0.0f;
GLuint programId;
GLint mvpLocation = 0;
int width = 800;
int height = 600;
float zoom = 0.577f; // Matches ~60-degree FOV (1.0f / 1.732f)
float eye[3] = {0.0f, 0.0f, 2.0f}; // Camera position
bool mouseDragging = false;
double lastX = 0.0, lastY = 0.0;

std::string vertexShader = R"(
    #version 140
    in vec3 pos;
    uniform mat4 mvp;
    void main()
    {
        gl_Position = mvp * vec4(pos, 1);
    }
)";

std::string fragmentShader = R"(
    #version 140
    void main() 
    {
        gl_FragColor = vec4(1, 0, 1, 1); // Magenta color
    }
)";

GLuint compileShaders(std::string shader, GLenum type)
{
    const char *shaderCode = shader.c_str();
    GLuint shaderId = glCreateShader(type);

    if (shaderId == 0)
    {
        std::cout << "Error creating shaders!" << std::endl;
        return 0;
    }

    glShaderSource(shaderId, 1, &shaderCode, NULL);
    glCompileShader(shaderId);

    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus)
    {
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char *cMessage = new char[length];
        glGetShaderInfoLog(shaderId, length, &length, cMessage);
        std::cout << "Cannot Compile Shader: " << cMessage << std::endl;
        delete[] cMessage;
        glDeleteShader(shaderId);
        return 0;
    }

    return shaderId;
}

GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId)
{
    programId = glCreateProgram();

    if (programId == 0)
    {
        std::cout << "Error Creating Shader Program" << std::endl;
        return 0;
    }

    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glBindAttribLocation(programId, 0, "pos");
    glLinkProgram(programId);

    GLint linkStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus)
    {
        std::cout << "Error Linking program" << std::endl;
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        glDeleteProgram(programId);
        return 0;
    }

    return programId;
}

// Scroll callback for zoom
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom -= yoffset * 0.1f;
    zoom = std::max(0.1f, std::min(zoom, 10.0f));
}

// Mouse button callback to detect dragging
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            mouseDragging = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE)
        {
            mouseDragging = false;
        }
    }
}

// Mouse movement callback to update camera position
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (mouseDragging)
    {
        double deltaX = xpos - lastX;
        double deltaY = ypos - lastY;
        eye[0] -= deltaX * 0.005f;
        eye[1] += deltaY * 0.005f;
        lastX = xpos;
        lastY = ypos;
    }
}

// Matrix multiplication function
void multiplyMatrix(float* result, float* a, float* b)
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            result[i*4+j] = 0.0f;
            for (int k = 0; k < 4; k++)
                result[i*4+j] += a[i*4+k] * b[k*4+j];
        }
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(width, height, "Cube Rotation with Scroll Zoom and Mouse Camera", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    GLuint vShaderId = compileShaders(vertexShader, GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader, GL_FRAGMENT_SHADER);
    programId = linkProgram(vShaderId, fShaderId);
    glUseProgram(programId);
    mvpLocation = glGetUniformLocation(programId, "mvp");

    // Vertices for plane and triangle (static)
    GLfloat planeTriangleVertices[] = {
        // Plane (two triangles forming a rectangle)
        -0.75f, -0.25f, 0.0f,
        -0.25f, -0.25f, 0.0f,
        -0.75f,  0.0f,  0.0f,
        -0.25f, -0.25f, 0.0f,
        -0.75f,  0.0f,  0.0f,
        -0.25f,  0.0f,  0.0f,
        // Additional Triangle
        0.0f,   -0.25f, 0.0f,
        0.25f,  -0.25f, 0.0f,
        0.0f,    0.0f,  0.0f
    };

    // Vertices for cube (rotating)
    GLfloat cubeVertices[] = {
        // Front face (Z = 0.0f)
        0.50f,  -0.25f, 0.0f,
        0.750f, -0.25f, 0.0f,
        0.50f,   0.0f,  0.0f,
        0.750f, -0.25f, 0.0f,
        0.50f,   0.0f,  0.0f,
        0.750f,  0.0f,  0.0f,
        // Back face (Z = 0.25f)
        0.50f,  -0.25f, 0.25f,
        0.75f,  -0.25f, 0.25f,
        0.50f,   0.0f,  0.25f,
        0.75f,  -0.25f, 0.25f,
        0.50f,   0.0f,  0.25f,
        0.75f,   0.0f,  0.25f,
        // Left face (X = 0.50f)
        0.50f,  -0.25f, 0.25f,
        0.50f,  -0.25f, 0.0f,
        0.50f,   0.0f,  0.25f,
        0.50f,  -0.25f, 0.0f,
        0.50f,   0.0f,  0.25f,
        0.50f,   0.0f,  0.0f,
        // Right face (X = 0.75f)
        0.75f,  -0.25f, 0.0f,
        0.75f,  -0.25f, 0.25f,
        0.75f,   0.0f,  0.0f,
        0.75f,  -0.25f, 0.25f,
        0.75f,   0.0f,  0.0f,
        0.75f,   0.0f,  0.25f,
        // Top face (Y = 0.0f)
        0.50f,   0.0f,  0.25f,
        0.75f,   0.0f,  0.25f,
        0.50f,   0.0f,  0.0f,
        0.75f,   0.0f,  0.25f,
        0.50f,   0.0f,  0.0f,
        0.75f,   0.0f,  0.0f,
        // Bottom face (Y = -0.25f)
        0.50f,  -0.25f, 0.0f,
        0.75f,  -0.25f, 0.0f,
        0.50f,  -0.25f, 0.25f,
        0.75f,  -0.25f, 0.0f,
        0.50f,  -0.25f, 0.25f,
        0.75f,  -0.25f, 0.25f
    };

    // Setup VAO and VBO for plane and triangle
    GLuint planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeTriangleVertices), planeTriangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Setup VAO and VBO for cube
    GLuint cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Projection matrix
        float aspect = (float)width / height;
        float fov = 1.0f / zoom;
        float near = 0.1f;
        float far = 100.0f;
        float proj[16] = {
            fov / aspect, 0.0f, 0.0f, 0.0f,
            0.0f, fov, 0.0f, 0.0f,
            0.0f, 0.0f, -(far + near)/(far - near), -1.0f,
            0.0f, 0.0f, -(2.0f * far * near)/(far - near), 0.0f
        };

        // View matrix: Camera at eye, looking at (0, 0, 0), up (0, 1, 0)
        float center[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 1.0f, 0.0f};

        float z[3] = {eye[0] - center[0], eye[1] - center[1], eye[2] - center[2]};
        float z_len = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);
        z[0] /= z_len; z[1] /= z_len; z[2] /= z_len;

        float x[3] = {up[1]*z[2] - up[2]*z[1], up[2]*z[0] - up[0]*z[2], up[0]*z[1] - up[1]*z[0]};
        float x_len = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
        x[0] /= x_len; x[1] /= x_len; x[2] /= x_len;

        float y[3] = {z[1]*x[2] - z[2]*x[1], z[2]*x[0] - z[0]*x[2], z[0]*x[1] - z[1]*x[0]};

        float view[16] = {
            x[0], y[0], z[0], 0.0f,
            x[1], y[1], z[1], 0.0f,
            x[2], y[2], z[2], 0.0f,
            -(x[0]*eye[0] + x[1]*eye[1] + x[2]*eye[2]),
            -(y[0]*eye[0] + y[1]*eye[1] + y[2]*eye[2]),
            -(z[0]*eye[0] + z[1]*eye[1] + z[2]*eye[2]),
            1.0f
        };

        // Rotation matrix for cube
        angle += 0.01f;
        float c = cosf(angle);
        float s = sinf(angle);
        float rot[16] = {
            c, 0.0f, s, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s, 0.0f, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Identity matrix for static objects (no rotation)
        float identity[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // MVP for plane and triangle: Proj * View * Identity
        float temp[16], mvp[16];
        multiplyMatrix(temp, proj, view);
        multiplyMatrix(mvp, temp, identity);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 9); // 9 vertices for plane + triangle

        // MVP for cube: Proj * View * Rot
        multiplyMatrix(temp, proj, view);
        multiplyMatrix(mvp, temp, rot);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices for cube

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteProgram(programId);
    glfwTerminate();

    return 0;
}