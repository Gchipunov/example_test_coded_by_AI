#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath> // Use cmath for math functions
// gcc -o bard_basic1 bard_basic1.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++


float angle = 0.0f; // Initial rotation angle
GLuint programId; // Shader program ID
GLint mvpLocation = 0; // Location of the MVP uniform in the shader
int width = 800; // Window width
int height = 600; // Window height


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
// parameters:
//   matrix: Pointer to a float array to store the resulting matrix (must be 16 elements for 4x4)
//   left, right: X-coordinates of the left and right clipping planes
//   bottom, top: Y-coordinates of the bottom and top clipping planes
//   near, far: Z-coordinates of the near and far clipping planes (distance from camera)
void createOrthographicMatrix(float* matrix, float left, float right, float bottom, float top, float near, float far) {
    // Initialize matrix to identity
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }
    matrix[0] = 2.0f / (right - left); // Scale X
    matrix[5] = 2.0f / (top - bottom); // Scale Y
    matrix[10] = -2.0f / (far - near); // Scale Z (inverted for typical OpenGL depth range)
    matrix[12] = -(right + left) / (right - left); // Translate X
    matrix[13] = -(top + bottom) / (top - bottom); // Translate Y
    matrix[14] = -(far + near) / (far - near); // Translate Z
    matrix[15] = 1.0f; // Homogeneous coordinate
}

int main(int argc, char** argv) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version (optional, but good practice for specific versions)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "Orthographic Camera Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Make the window's context current

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
    // Calculate the total number of vertices: (6 * 2 for quads) + 3 (triangle) + (6 * 6 for cube) + (6 for main rect) + (6 * 4 for small rects)
    // Triangle 1 (quad): 2 triangles * 3 vertices/triangle = 6 vertices
    // Triangle 2: 1 triangle * 3 vertices/triangle = 3 vertices
    // Cube: 6 faces * 2 triangles/face * 3 vertices/triangle = 36 vertices
    // Main Bottom Rectangle: 2 triangles * 3 vertices/triangle = 6 vertices
    // Rectangle group (4 rectangles): 4 rectangles * 2 triangles/rectangle * 3 vertices/triangle = 24 vertices
    const int vertexCount = 6 + 3 + 36 + 6 + 24; // Total = 75 vertices

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

        // Orthographic projection matrix
        float proj[16];
        // Define the orthographic viewing volume.
        // These values are chosen to encompass the objects within the -1.0 to 1.0 range (approximately)
        // and a reasonable depth range.
        float orthoLeft = -1.0f * ((float)width / height); // Scale left/right by aspect ratio
        float orthoRight = 1.0f * ((float)width / height);
        float orthoBottom = -1.0f;
        float orthoTop = 1.0f;
        float orthoNear = 0.1f;  // Near plane
        float orthoFar = 100.0f; // Far plane (objects are effectively at Z=-7.0 to -6.75 after translation)
                                // So this range [-6.0, -8.0] should cover them.
        createOrthographicMatrix(proj, orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);

        // Rotation matrix (slight rotation around Y-axis to see the cube's 3D nature)
        angle += 0.005f; // Continuously rotate
        float c = cosf(angle);
        float s = sinf(angle);
        float rot[16] = {
            c,    0.0f, s,    0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            -s,   0.0f, c,    0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Translation matrix (move objects back into the viewing volume of the orthographic camera)
        // With an orthographic camera, objects don't shrink with distance.
        // We move them back so their Z-values fall within the near/far planes of the orthographic projection.
        // Since the projection is now from Z=-0.1 to Z=-100.0 (effectively, after negating Z for OpenGL's depth),
        // we can place objects directly in front of the camera, or further back.
        // A smaller negative Z here means they are closer to the camera.
        float trans[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, -2.0f, // Translate slightly back along Z to position objects
            0.0f, 0.0f, 0.0f, 1.0f
        };

        // Compute MVP matrix: proj * trans * rot
        // Note: The order of multiplication depends on what you want to apply first.
        // Typically, model (rotation/scale) -> view (camera position) -> projection.
        // Here, 'rot' is acting as a model transformation and 'trans' as a view translation.
        // So, it's projection * view * model (in reverse order for matrix multiplication).
        float temp[16], mvp[16];
        // Multiply projection * rotation
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    temp[i*4+j] += proj[i*4+k] * rot[k*4+j];
            }
        }
        // Multiply (proj * rot) * translation
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                mvp[i*4+j] = 0.0f;
                for (int k = 0; k < 4; k++)
                    mvp[i*4+j] += temp[i*4+k] * trans[k*4+j];
            }
        }

        // Apply the computed MVP matrix to the shader uniform
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);

        // Draw objects
        glBindVertexArray(vao); // Bind the VAO containing vertex data
        glDrawArrays(GL_TRIANGLES, 0, vertexCount); // Draw all vertices as triangles

        // Swap front and back buffers (double buffering)
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
