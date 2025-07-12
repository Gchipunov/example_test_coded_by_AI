#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath> // Use cmath for math functions
// gcc -o bard_basic4 bard_basic4.cpp -lglfw3 -lglu32 -lopengl32 -lpthread -lglew32 -lgdi32 -lstdc++ -I"C:\android_pc_game\scenecode"

// Include GLM for vector and matrix mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- Global Variables ---
// Window dimensions
int width = 800;
int height = 600;

// Shader program ID and MVP uniform location
GLuint programId;
GLint mvpLocation = 0;

// Camera state
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);   // Camera starts at (0,0,3)
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera looks towards negative Z
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);    // Up direction is Y-axis

float yaw = -90.0f;  // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right.
float pitch = 0.0f;
float lastX = (float)width / 2.0f; // Initial mouse position for first calculation
float lastY = (float)height / 2.0f;
bool firstMouse = true; // Flag to handle initial mouse jump

float fov = 45.0f; // Field of View for perspective camera

float cameraSpeed = 2.5f; // Camera movement speed
float mouseSensitivity = 0.1f; // Mouse rotation sensitivity

// Timing for frame-rate independent movement
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Input key states
bool keys[1024];

// Camera projection mode
bool isOrthographic = false; // Default to perspective camera initially

// Object rotation angle (for the cube, triangles etc.)
float objectAngle = 0.0f;

// --- Shader Source Code ---
std::string vertexShader = R"(
    #version 140
    in vec3 pos;
    in vec4 attr_color;
    uniform mat4 mvp;
    out vec4 v_attr_color;
    void main() {
        v_attr_color = attr_color;
        gl_Position = mvp * vec4(pos, 1.0);
    }
)";

std::string fragmentShader = R"(
    #version 140
    in vec4 v_attr_color;
    void main() {
        gl_FragColor = v_attr_color;
    }
)";

// --- Shader Compilation and Linking Functions (Unchanged) ---
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

// --- Input Callback Functions ---
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) { // Check if key is within valid range
        if (action == GLFW_PRESS) {
            keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }

    // Camera projection mode switch
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_O) {
            isOrthographic = true;
            std::cout << "Switched to Orthographic Camera (Press P for Perspective)" << std::endl;
        } else if (key == GLFW_KEY_P) {
            isOrthographic = false;
            std::cout << "Switched to Perspective Camera (Press O for Orthographic)" << std::endl;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to avoid camera flipping
    if (pitch > 189.0f) {
        pitch = 189.0f;
    }
    if (pitch < -189.0f) {
        pitch = -189.0f;
    }

    // Calculate new front vector from updated yaw and pitch
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Function to handle continuous input based on key states
void processInput() {
    float velocity = cameraSpeed * deltaTime;
    if (keys[GLFW_KEY_W]) {
        cameraPos += cameraFront * velocity;
    }
    if (keys[GLFW_KEY_S]) {
        cameraPos -= cameraFront * velocity;
    }
    if (keys[GLFW_KEY_A]) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    }
    if (keys[GLFW_KEY_D]) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * velocity;
    }
	 if (keys[GLFW_KEY_L]) {
//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);     
//	 cameraPos = glm::lookAt(cameraDirection);
//glm::mat4 view;
cameraPos = glm::vec3(0.0f, 0.0f, 0.0f) ;//glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), 
  		  // glm::vec3(0.0f, 0.0f, 0.0f), 
  		  // glm::vec3(0.0f, 1.0f, 0.0f));
    }
}


int main(int argc, char** argv) {
    // --- Initialize GLFW ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  //  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    // --- Create Window ---
    GLFWwindow* window = glfwCreateWindow(width, height, "Camera Movement Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- Set Input Callbacks ---
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Disable and hide mouse cursor for camera control
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --- Initialize GLEW ---
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // --- Compile and Link Shaders ---
    GLuint vShaderId = compileShaders(vertexShader, GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader, GL_FRAGMENT_SHADER);
    programId = linkProgram(vShaderId, fShaderId);
    glUseProgram(programId);
    mvpLocation = glGetUniformLocation(programId, "mvp");

    // --- Vertex Data ---
    // Combined vertex data (position and color) for all shapes
    // Each vertex: 3 floats for position (x, y, z), 4 floats for color (r, g, b, a)
    GLfloat vertices[] = {
        // Triangle 1 (top-left, flat in Z=0 plane) - Reddish
        -0.75f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.25f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.75f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.25f, -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right (second triangle for quad)
        -0.75f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left (second triangle for quad)
        -0.25f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right (second triangle for quad)

        // Triangle 2 (middle, flat in Z=0 plane) - Reddish
        0.0f,   -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        0.25f,  -0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        0.0f,    0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left

        // Cube (all faces, green/blue/reddish for different faces)
        // Front face (Z=0.0f) - Greenish
        0.50f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-left
        0.75f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-left
        0.75f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-left
        0.75f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.7f, // Top-right
        // Back face (Z=0.25f) - Blueish
        0.50f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-left
        0.75f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-left
        0.75f, -0.25f, 0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Bottom-right
        0.50f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-left
        0.75f,  0.0f,  0.25f, 0.0f, 0.0f, 1.0f, 0.7f, // Top-right
        // Left face (X=0.50f) - Reddish
        0.50f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.50f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.50f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        0.50f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.50f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        0.50f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        // Right face (X=0.75f) - Reddish
        0.75f, -0.25f, 0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Bottom-front
        0.75f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.75f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        0.75f, -0.25f, 0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-back
        0.75f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 0.7f, // Top-front
        0.75f,  0.0f,  0.25f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-back
        // Top face (Y=0.0f) - Greenish
        0.50f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right
        0.50f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f,  0.0f,  0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right
        0.50f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        // Bottom face (Y=-0.25f) - Greenish
        0.50f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-left
        0.75f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        0.50f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f, -0.25f, 0.0f,  0.0f, 1.0f, 0.0f, 0.7f, // Front-right
        0.50f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-left
        0.75f, -0.25f, 0.25f, 0.0f, 1.0f, 0.0f, 0.7f, // Back-right

        // Main Bottom Rectangle (blackish)
        -0.75f, -0.75f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.25f, -0.75f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.75f, -0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.25f, -0.75f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.75f, -0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.25f, -0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, // Top-right

        // Rectangle group (flat in Z=0.1f plane, reddish)
        // Rectangle 1
        -0.70f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.64f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.70f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.64f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.70f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.64f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 2
        -0.61f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.57f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.61f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.57f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.61f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.57f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 3
        -0.54f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.50f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.54f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.50f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.54f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.50f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-right
        // Rectangle 4
        -0.47f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-left
        -0.41f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.47f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.41f, -0.72f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Bottom-right
        -0.47f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f, // Top-left
        -0.41f, -0.53f, 0.1f, 1.0f, 0.0f, 0.0f, 0.7f  // Top-right
    };
    const int vertexCount = sizeof(vertices) / (7 * sizeof(GLfloat));

    // --- VAO and VBO Setup ---
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --- OpenGL State ---
  //  glEnable(GL_DEPTH_TEST);
  //  glDepthFunc(GL_LESS);
glDisable(GL_CULL_FACE);



    // --- Main Rendering Loop ---
    while (!glfwWindowShouldClose(window)) {
        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process keyboard input for camera movement
        processInput();

        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Camera Matrices (Projection, View, Model) ---
        glm::mat4 projection;
        if (isOrthographic) {
            // Orthographic projection
            float aspect = (float)width / height;
            float orthoSize = 1.25f; // Adjust this value to control "zoom" in orthographic view
            projection = glm::ortho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, 0.1f, 100.0f);
        } else {
            // Perspective projection
            projection = glm::perspective(glm::radians(fov), (float)width / height, 0.1f, 100.0f);
        }

        // View matrix (camera position and orientation)
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Model matrix (for the objects themselves, e.g., cube rotation)
        // We'll keep a subtle rotation on the objects to make them look more dynamic
        // and to better observe the camera movement.
        objectAngle += 0.5f * deltaTime; // Rotate slowly
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(objectAngle * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis rotation
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f)); // Move objects slightly back

        // Combine matrices: Projection * View * Model
        glm::mat4 mvp = projection * view * model;

        // Apply MVP matrix to shader
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        // Draw objects
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        // Swap buffers and poll events
      //  glfwSwapBuffers(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(programId);
    glfwTerminate();
    return 0;
}