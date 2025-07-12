#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <math.h>

// gcc -o basic2 basic2.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++


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
        // Scale mouse movement to camera position (adjust sensitivity with 0.005f)
        eye[0] -= deltaX * 0.005f;
        eye[1] += deltaY * 0.005f; // Invert Y to match screen coordinates
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

    GLFWwindow* window = glfwCreateWindow(width, height, "Basic Shapes with Scroll Zoom and Mouse Camera", NULL, NULL);
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

    // Define the vertices (plane, triangle, cube)
    GLfloat vertices[] = {
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
        0.0f,    0.0f,  0.0f,
        // Cube
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

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
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

        // Rotation matrix (model transformation)
        angle += 0.01f;
        float c = cosf(angle);
        float s = sinf(angle);
        float rot[16] = {
            c, 0.0f, s, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s, 0.0f, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Combine matrices: MVP = Proj * View * Rot
        float temp[16], mvp[16];
        multiplyMatrix(temp, proj, view);
        multiplyMatrix(mvp, temp, rot);

        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 45);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(programId);
    glfwTerminate();

    return 0;
}