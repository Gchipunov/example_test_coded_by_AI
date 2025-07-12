#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath> // Use cmath for math functions

// Global variables for camera control
float angle = 0.0f; // Initial rotation angle for objects
GLuint programId; // Shader program ID
GLint mvpLocation = 0; // Location of the MVP uniform in the shader
int width = 800; // Window width
int height = 600; // Window height
bool isOrthographic = true; // Flag to determine current camera mode (true for orthographic, false for perspective)


// Command to compile:
// gcc -o grok_basic2 grok_basic2.cpp -lglfw3 -lglu32 -lopengl32 -lpthread -lglew32 -lgdi32 -lstdc++


// Vertex shader with corrected matrix multiplication order
// It receives position and color attributes and applies the MVP matrix.
std::string vertexShader = R"(
    #version 140
    in vec3 pos; // Input vertex position
    in vec4 attr_color; // Input vertex color
    uniform mat4 mvp; // Model-View-Projection matrix
    out vec4 v_attr_color; // Output color to fragment shader
    void main() {
        v_attr_color = attr_color; // Pass color through to fragment shader
        gl_Position = mvp * vec4(pos, 1.0); // Apply MVP matrix to vertex position
    }
)";

// Fragment shader
// It receives interpolated color and outputs it as the final fragment color.
std::string fragmentShader = R"(
    #version 140
    in vec4 v_attr_color; // Input interpolated color from vertex shader
    void main() {
        gl_FragColor = v_attr_color; // Set the fragment color
    }
)";

// Compile shader function
// Compiles a single shader (vertex or fragment) and checks for compilation errors.
GLuint compileShaders(const std::string& shader, GLenum type) {
    const char* shaderCode = shader.c_str(); // Get C-style string from std::string
    GLuint shaderId = glCreateShader(type); // Create a shader object
    if (shaderId == 0) {
        std::cout << "Error creating shader!" << std::endl;
        return 0;
    }
    glShaderSource(shaderId, 1, &shaderCode, nullptr); // Set shader source code
    glCompileShader(shaderId); // Compile the shader
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus); // Check compilation status
    if (!compileStatus) {
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length); // Get error log length
        char* cMessage = new char[length];
        glGetShaderInfoLog(shaderId, length, &length, cMessage); // Get error log
        std::cout << "Cannot Compile Shader: " << cMessage << std::endl;
        delete[] cMessage; // Clean up allocated memory
        glDeleteShader(shaderId); // Delete shader object
        return 0;
    }
    return shaderId;
}

// Link program function
// Links vertex and fragment shaders into a shader program and checks for linking errors.
GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId) {
    programId = glCreateProgram(); // Create a shader program object
    if (programId == 0) {
        std::cout << "Error Creating Shader Program" << std::endl;
        return 0;
    }
    glAttachShader(programId, vertexShaderId); // Attach vertex shader
    glAttachShader(programId, fragmentShaderId); // Attach fragment shader
    // Bind attribute locations to specific indices (pos to 0, attr_color to 1)
    glBindAttribLocation(programId, 0, "pos");
    glBindAttribLocation(programId, 1, "attr_color");
    glLinkProgram(programId); // Link the program
    GLint linkStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus); // Check linking status
    if (!linkStatus) {
        std::cout << "Error Linking program" << std::endl;
        // Detach and delete shaders/program on error
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        glDeleteProgram(programId);
        return 0;
    }
    return programId;
}

// Helper function to create an orthographic projection matrix
// This function constructs a 4x4 orthographic projection matrix.
void createOrthographicMatrix(float* matrix, float left, float right, float bottom, float top, float near, float far) {
    // Initialize matrix to identity (or zeros, then fill)
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }
    // Set up the diagonal elements and translation components
    matrix[0] = 2.0f / (right - left); // Scale X
    matrix[5] = 2.0f / (top - bottom); // Scale Y
    matrix[10] = -2.0f / (far - near); // Scale Z (inverted for typical OpenGL depth range)
    matrix[12] = -(right + left) / (right - left); // Translate X
    matrix[13] = -(top + bottom) / (top - bottom); // Translate Y
    matrix[14] = -(far + near) / (far - near); // Translate Z
    matrix[15] = 1.0f; // Homogeneous coordinate
}

// Helper function to create perspective projection matrix
// This function constructs a 4x4 perspective projection matrix.
void createPerspectiveMatrix(float* matrix, float fov, float aspect, float near, float far) {
    // Initialize matrix to identity (or zeros, then fill)
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }
    float tanHalfFov = tan(fov / 2.0f);
    matrix[0] = 1.0f / (aspect * tanHalfFov);
    matrix[5] = 1.0f / tanHalfFov;
    matrix[10] = -(far + near) / (far - near);
    matrix[11] = -1.0f;
    matrix[14] = -(2.0f * far * near) / (far - near);
    matrix[15] = 0.0f; // For perspective, the last element is usually 0
}

// Keyboard callback function
// This function is called by GLFW when a key event occurs.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) { // Only react on key press (not repeat or release)
        if (key == GLFW_KEY_O) {
            isOrthographic = true; // Switch to orthographic camera
            std::cout << "Switched to Orthographic Camera" << std::endl;
        } else if (key == GLFW_KEY_P) {
            isOrthographic = false; // Switch to perspective camera
            std::cout << "Switched to Perspective Camera" << std::endl;
        }
    }
}

int main(int argc, char** argv) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (optional, but good practice for specific versions)
    // Core profile means no deprecated functionality
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 //   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for macOS

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "Camera Switch Example (O/P)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Make the window's context current

    // Set the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLEW
    // GLEW must be initialized after a valid OpenGL rendering context has been created
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Compile and link shaders
    GLuint vShaderId = compileShaders(vertexShader, GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader, GL_FRAGMENT_SHADER);
    programId = linkProgram(vShaderId, fShaderId);
    glUseProgram(programId); // Use the created shader program
    mvpLocation = glGetUniformLocation(programId, "mvp"); // Get location of MVP uniform

    // Optimized vertex data: triangles, cube, and rectangles with consistent planes
    // Each vertex has 3 position components (x, y, z) and 4 color components (r, g, b, a)
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
    // Calculate the total number of vertices based on the array size
    // Each vertex has 7 float components (3 for position, 4 for color)
    const int vertexCount = sizeof(vertices) / (7 * sizeof(GLfloat));

    // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao); // Generate VAO
    glGenBuffers(1, &vbo); // Generate VBO
    glBindVertexArray(vao); // Bind VAO (all subsequent VBO/attribute calls are stored in this VAO)
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind VBO
    // Load vertex data into VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position attribute (layout 0): 3 floats, starting at offset 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    // Color attribute (layout 1): 4 floats, starting at offset 3 * sizeof(GLfloat)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Unbind VBO and VAO to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable depth testing to ensure correct rendering of 3D objects (objects closer to camera hide objects further away)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Specifies the depth comparison function

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Set background color (dark teal)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

        float proj[16]; // Projection matrix
        // Determine which projection matrix to use based on the global flag
        if (isOrthographic) {
            // Orthographic projection parameters:
            // left, right, bottom, top define the visible area in world coordinates.
            // near, far define the depth range.
            // Aspect ratio is applied to match window dimensions.
            float orthoLeft = -1.0f * ((float)width / height);
            float orthoRight = 1.0f * ((float)width / height);
            float orthoBottom = -1.0f;
            float orthoTop = 1.0f;
            float orthoNear = 0.1f;
            float orthoFar = 100.0f; // Needs to be large enough to contain translated objects
            createOrthographicMatrix(proj, orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
        } else {
            // Perspective projection parameters:
            // FOV (Field of View), Aspect Ratio, Near plane, Far plane.
            float fov = 60.0f * (3.14159f / 180.0f); // 60 degrees in radians
            float aspect = (float)width / height;
            float near = 0.1f;
            float far = 100.0f;
            createPerspectiveMatrix(proj, fov, aspect, near, far);
        }

        // Rotation matrix (slight rotation around Y-axis to see the cube's 3D nature)
        angle += 0.005f; // Continuously rotate for animation
        float c = cosf(angle);
        float s = sinf(angle);
        float rot[16] = {
            c,    0.0f, s,    0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s,   0.0f, c,    0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Translation matrix (move objects back to be visible)
        // The Z translation value is important for both camera types.
        // For perspective, it controls apparent size. For orthographic, it just places objects
        // within the clipping planes without affecting size.
        float trans[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, -2.0f, // Translate slightly back along Z axis
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Compute MVP matrix: proj * trans * rot
        // The order of matrix multiplication is crucial: apply model, then view, then projection.
        // In column-major OpenGL, this means multiplying from right to left: Projection * View * Model.
        // Here, 'rot' is Model, 'trans' is View (camera translation), 'proj' is Projection.
        float temp[16], mvp[16];
        // Multiply Model (rot) by View (trans) to get ModelView matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    temp[i*4+j] += trans[i*4+k] * rot[k*4+j];
            }
        }
        // Multiply Projection (proj) by ModelView (temp) to get MVP matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mvp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    mvp[i*4+j] += proj[i*4+k] * temp[k*4+j];
            }
        }

        // Apply the computed MVP matrix to the shader uniform
        // GL_FALSE means the matrix is column-major (which our matrices are)
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);

        // Draw objects
        glBindVertexArray(vao); // Bind the VAO containing vertex data
     //   glDrawArrays(GL_TRIANGLES, 0, vertexCount); // Draw all vertices as triangles
 glDrawArrays(GL_TRIANGLES, 0, 75); // Draw all vertices as triangles

        // Swap front and back buffers (double buffering for smooth animation)
        glfwSwapBuffers(window);
        // Process any pending events (e.g., window resize, keyboard input)
        glfwPollEvents();
    }

    // Cleanup: Delete OpenGL objects and terminate GLFW
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(programId);
    glfwTerminate(); // Terminate GLFW
    return 0;
}