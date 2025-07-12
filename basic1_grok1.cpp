// georgiy chipunov
// colo game test engine
// D:
// cd D:\games_src\example_test_coded_by_AI
// gcc -o meta_glew_glfw3_1 meta_glew_glfw3_1.cpp -lglfw3 -lGLEW -lGL -lGLU -lpthread -ldl
// ./meta_glew_glfw3_1

// gcc -o meta_glew_glfw3_1 meta_glew_glfw3_1.cpp -lglfw3 -L"C:/mingw64/x86_64-w64-mingw32/lib/" -lglew  -lglu32 -lopengl32 -lpthread -ldl

// gcc -o meta_glew_glfw3_1 meta_glew_glfw3_1.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++

// NEXT UPGRADE VULKAN

// https://github.com/jonathan-slark/vulkan-triangle
// https://github.com/lonelydevil/vulkan-tutorial-C-implementation
// https://github.com/lonelydevil/vulkan-tutorial-C-implementation
// https://stackoverflow.com/questions/49872156/rendering-a-cube-in-vulkan-vs-opengl

//g++ -o vulkan_opengl_example vulkan_opengl_example.c -lvulkan -lopengl32 -lglew32 -lglfw


/*
Combining Vulkan and OpenGL3 rendering in a single project with MinGW32 in C can be quite complex, as these APIs are fundamentally different. However, you can set up a project where both APIs are used for rendering, such as rendering a triangle with Vulkan and OpenGL3. Here's a high-level guide:

1. Set Up Your Environment


2. Write the Vulkan and OpenGL Code


3. Compile the Code


4. Run the Example


For detailed implementations, you can explore this Vulkan tutorial or this OpenGL example. Let me know if you'd like help with specific parts of the setup or code! 
*/
// -L"D:/OpenAL_1-1_SDK/libs/Win64"


// gcc -o meta_glew_glfw3_basic_1 meta_glew_glfw3_basic_1.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
 //gcc -o basic1_grok1 basic1_grok1.cpp -lglfw3 -lglu32 -lopengl32 -lpthread   -lglew32   -lgdi32 -lstdc++

#include <math.h>

float angle = 0.0f;
GLuint programId;

GLint mvpLocation = 0;
	int width= 800;
	int height = 600;
	
float zoom = 0.577f; // Matches ~60-degree FOV (1.0f / 1.732f)

//float fov = 1.0f / zoom;
//float near = 0.1f;
//float near = -0.1f;
//float far = 100.0f;
//float far = 1.0f;

//float fov = 1.732f  / zoom;; // Corresponding to 60 degrees FOV
//float near = 0.1f;
//float far = 100.0f;


	std::string vertexShader = R"(
    #version 140
    in vec3 pos;
	uniform mat4 mvp;
    void main()
    {
        gl_Position = vec4(pos, 1) * mvp ;
    }
)";

std::string fragmentShader = R"(
    #version 140
    void main() 
    {
        gl_FragColor = vec4(1, 0, 1, 1); // white color
    }
)";

// Compile and create shader object and returns its id
GLuint compileShaders(std::string shader, GLenum type)
{

    const char *shaderCode = shader.c_str();
    GLuint shaderId = glCreateShader(type);

    if (shaderId == 0)
    { // Error: Cannot create shader object
        std::cout << "Error creating shaders!";
        return 0;
    }

    // Attach source code to this object
    glShaderSource(shaderId, 1, &shaderCode, NULL);
    glCompileShader(shaderId); // compile the shader object

    GLint compileStatus;

    // check for compilation status
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus)
    { // If compilation was not successfull
        int length;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
        char *cMessage = new char[length];

        // Get additional information
        glGetShaderInfoLog(shaderId, length, &length, cMessage);
        std::cout << "Cannot Compile Shader: " << cMessage;
        delete[] cMessage;
        glDeleteShader(shaderId);
        return 0;
    }

    return shaderId;
}

// Creates a program containing vertex and fragment shader
// links it and returns its ID
GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId)
{
     programId = glCreateProgram(); // crate a program

    if (programId == 0)
    {
        std::cout << "Error Creating Shader Program";
        return 0;
    }

    // Attach both the shaders to it
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
glBindAttribLocation(programId, 0, "pos");

	
    // Create executable of this program
    glLinkProgram(programId);

    GLint linkStatus;

    // Get the link status for this program
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus)
    { // If the linking failed
        std::cout << "Error Linking program";
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        glDeleteProgram(programId);

        return 0;
    }

    return programId;
}

// Scroll callback function for zoom
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom -= yoffset * 0.1f; // Adjust zoom based on scroll direction
    zoom = std::max(0.001f, std::min(zoom, 100.0f)); // Clamp zoom between 0.1 and 10
	printf("Zoom: %f",(float)yoffset);
	
}
// Simple matrix multiplication function
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
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Create a windowed mode window and its OpenGL context
  GLFWwindow* window = glfwCreateWindow(width, height, "Basic Shapes Examples", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
// Set scroll callback
    glfwSetScrollCallback(window, scrollCallback);


  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }


  GLuint vShaderId = compileShaders(vertexShader.c_str(), GL_VERTEX_SHADER);
    GLuint fShaderId = compileShaders(fragmentShader.c_str(), GL_FRAGMENT_SHADER);
	
     programId = linkProgram(vShaderId, fShaderId);
	  glUseProgram(programId);
	  
 mvpLocation = glGetUniformLocation(programId, "mvp");

	
  // Define the triangle vertices
  GLfloat vertices[] = {
	                      //() = tryAng_________le
    -0.75f,	-0.25f,    0.0f,   //    (1)    |      /  (2)
	-0.25f ,   -0.25f, 0.0f,   //           |   /      
	  -0.75f,   0.0f,   0.0f,  //       (3) |/         (4)
	 -0.25f ,   -0.25f, 0.0f,  // (2) 
	    -0.75f,   0.0f,0.0f,   // (3)
	  -0.25f , 0.0f,  0.0f,    // (4)
	  
    0.0f, -0.25f,     0.0f,    // Triangle Vertex (1)   Bottom Left
     0.25f, -0.25f, 0.0f,      // Triangle Vertex (2)   Bottom Right
     0.0f,  0.0f  ,   0.0f,     // Triangle Vertex (3)   Top Left
	 
	 // Cube
		0.50f, -0.25f, 0.0f,     //(1)
		0.750f, -0.25f, 0.0f,    //(2) 
		0.50f,  0.0f,  0.0f,     //(3)
 
		0.750f, -0.25f, 0.0f,   //(4)
		0.50f,  0.0f, 0.0f,     //(5)
		0.750f,0.0f,  0.0f,     //(6)
		
		 // Back face (Z = 0.25f)
    0.50f, -0.25f, 0.25f, // Bottom-left
    0.75f, -0.25f, 0.25f, // Bottom-right
    0.50f,  0.0f,  0.25f, // Top-left
    0.75f, -0.25f, 0.25f, // Bottom-right
    0.50f,  0.0f,  0.25f, // Top-left
    0.75f,  0.0f,  0.25f, // Top-right

    // Left face (X = 0.50f)
    0.50f, -0.25f, 0.25f, // Bottom-back
    0.50f, -0.25f, 0.0f,  // Bottom-front
    0.50f,  0.0f,  0.25f, // Top-back
    0.50f, -0.25f, 0.0f,  // Bottom-front
    0.50f,  0.0f,  0.25f, // Top-back
    0.50f,  0.0f,  0.0f,  // Top-front

    // Right face (X = 0.75f)
    0.75f, -0.25f, 0.0f,  // Bottom-front
    0.75f, -0.25f, 0.25f, // Bottom-back
    0.75f,  0.0f,  0.0f,  // Top-front
    0.75f, -0.25f, 0.25f, // Bottom-back
    0.75f,  0.0f,  0.0f,  // Top-front
    0.75f,  0.0f,  0.25f, // Top-back

    // Top face (Y = 0.0f)
    0.50f,  0.0f,  0.25f, // Back-left
    0.75f,  0.0f,  0.25f, // Back-right
    0.50f,  0.0f,  0.0f,  // Front-left
    0.75f,  0.0f,  0.25f, // Back-right
    0.50f,  0.0f,  0.0f,  // Front-left
    0.75f,  0.0f,  0.0f,  // Front-right

    // Bottom face (Y = -0.25f)
    0.50f, -0.25f, 0.0f,  // Front-left
    0.75f, -0.25f, 0.0f,  // Front-right
    0.50f, -0.25f, 0.25f, // Back-left
    0.75f, -0.25f, 0.0f,  // Front-right
    0.50f, -0.25f, 0.25f, // Back-left
    0.75f, -0.25f, 0.25f  // Back-right
  };

  // Create a VAO and VBO
  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  // Bind the VAO and VBO
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // Fill the VBO with the triangle vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Specify the vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


// Simple perspective projection old
 //   float aspect = (float)width / height;
 float aspect = (float)width / height;
  //  float proj[16] = {
   //     1.0f/aspect, 0.0f, 0.0f, 0.0f,
  //      0.0f, 1.0f, 0.0f, 0.0f,
  //      0.0f, 0.0f, -1.01f, -1.0f,
   //     0.0f, 0.0f, -2.01f, 0.0f
   // };
   float fov = 1.0f / zoom;
        float near = 0.1f;
        float far = 100.0f;

float proj[16] = {
    fov/aspect, 0.0f, 0.0f, 0.0f,
    0.0f, fov, 0.0f, 0.0f,
    0.0f, 0.0f, -(far+near)/(far-near), -1.0f,
    0.0f, 0.0f, -(2.0f*far*near)/(far-near), 0.0f
};
 //float proj[16] = {
 //       1.0f/aspect, 0, 0, -1,
 //       0, 1.0f, 0, 1,
 //       0, 0, -1, 0,
 //       0, 0, 0, 1
  //  }; 
	// View matrix: Camera at (0, 0, 2), looking at (0, 0, 0), up (0, 1, 0)
        float eye[3] = {0.0f, 10.0f, 0.0f}; // Camera position
        float center[3] = {0.0f, 0.0f, 0.0f}; // Look-at point
        float up[3] = {0.0f, 1.0f, 0.0f}; // Up vector

        // Compute view matrix (look-at)
        float z[3] = {eye[0] - center[0], eye[1] - center[1], eye[2] - center[2]};
        float z_len = sqrt(z[0]*z[0] + z[1]*z[1] + z[2]*z[2]);
        z[0] /= z_len; z[1] /= z_len; z[2] /= z_len; // Normalize z

        float x[3] = {up[1]*z[2] - up[2]*z[1], up[2]*z[0] - up[0]*z[2], up[0]*z[1] - up[1]*z[0]};
        float x_len = sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
        x[0] /= x_len; x[1] /= x_len; x[2] /= x_len; // Normalize x

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

    // Rotation matrix
   // angle += 0.01f;
   angle += 0.11f;
    float c = cosf(angle);
    float s = sinf(angle);
    float rot[16] = {
        c, 0.0f, s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -s, 0.0f, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Translate cube away from camera
  /*  float trans[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, -2.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Matrix multiplication
    float mvp[16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            mvp[i*4+j] = 0.0f;
            for (int k = 0; k < 4; k++)
                mvp[i*4+j] += proj[i*4+k] * rot[k*4+j];
        }

  float temp[16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            temp[i*4+j] = 0.0f;
            for (int k = 0; k < 4; k++)
                temp[i*4+j] += mvp[i*4+k] * trans[k*4+j];
        }
    for (int i = 0; i < 16; i++)
        mvp[i] = temp[i];
*/
// Combine matrices: MVP = Proj * View * Rot
        float temp[16], mvp[16];
        multiplyMatrix(temp, proj, view);
        multiplyMatrix(mvp, temp, rot);
		
    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);
//mvp

    // Draw the triangle
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 45);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Cleanup
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glfwTerminate();

  return 0;
}