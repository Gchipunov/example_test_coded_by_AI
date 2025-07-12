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

#include <math.h>

float angle = 0.0f;
GLuint programId;

GLint mvpLocation = 0;
	int width= 800;
	int height = 600;
	
	std::string vertexShader = R"(
    #version 140
    in vec3 pos;
	in vec4 attr_color;
	uniform mat4 mvp;
	
	out vec4 v_attr_color;
	
    void main()
    {
		v_attr_color = attr_color;
        gl_Position = vec4(pos, 1) * mvp ;
    }
)";
 // uniform mat4 mvp;
std::string fragmentShader = R"(
    #version 140
	in vec4 v_attr_color;
	
    void main() 
    {
        gl_FragColor = v_attr_color ; //vec4(1, 0, 1, 1); // white color
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

glBindAttribLocation(programId, 1, "attr_color");

	
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

	// 1.0f,0.0f,0.0f ,0.7f,
  // Define the triangle vertices
  GLfloat vertices[] = { 
	                      //() = tryAng_________le
    -0.75f,	-0.25f,    0.0f,  1.0f,0.0f,0.0f ,0.7f, //    (1)    |      /  (2)
	-0.25f ,   -0.25f, 0.0f, 1.0f,0.0f,0.0f ,0.7f,  //           |   /      
	  -0.75f,   0.0f,   0.0f, 1.0f,0.0f,0.0f ,0.7f, //       (3) |/         (4)
	 -0.25f ,   -0.25f, 0.0f,  1.0f,0.0f,0.0f ,0.7f,// (2) 
	    -0.75f,   0.0f,0.0f,  1.0f,0.0f,0.0f ,0.7f, // (3)
	  -0.25f , 0.0f,  0.0f,   1.0f,0.0f,0.0f ,0.7f, // (4)
	  
    0.0f, -0.25f,     0.0f, 1.0f,0.0f,0.0f ,0.7f,   // Triangle Vertex (1)   Bottom Left
     0.25f, -0.25f, 0.0f,   1.0f,0.0f,0.0f ,0.7f,   // Triangle Vertex (2)   Bottom Right
     0.0f,  0.0f  ,   0.0f,  1.0f,0.0f,0.0f ,0.7f,   // Triangle Vertex (3)   Top Left
	 
	 // Cube
		0.50f, -0.25f, 0.0f, 0.0f,1.0f,0.0f ,0.7f,    //(1)
		0.750f, -0.25f, 0.0f, 0.0f,0.0f,1.0f ,0.7f,   //(2) 
		0.50f,  0.0f,  0.0f,  0.0f,0.0f,0.0f ,0.7f,   //(3)
 
		0.750f, -0.25f, 0.0f, 0.0f,1.0f,0.0f ,0.7f,  //(4)
		0.50f,  0.0f, 0.0f,   0.0f,0.0f,1.0f ,0.7f,  //(5)
		0.750f,0.0f,  0.0f,   0.0f,0.0f,0.0f ,0.7f,  //(6)
		
		 // Back face (Z = 0.25f)
    0.50f, -0.25f, 0.25f, 1.0f,1.0f,1.0f ,0.7f,// Bottom-left
    0.75f, -0.25f, 0.25f, 1.0f,1.0f,1.0f ,0.7f,// Bottom-right
    0.50f,  0.0f,  0.25f, 1.0f,1.0f,1.0f ,0.7f,// Top-left
    0.75f, -0.25f, 0.25f, 1.0f,1.0f,0.0f ,0.7f,// Bottom-right
    0.50f,  0.0f,  0.25f, 0.0f,1.0f,0.0f ,0.7f,// Top-left
    0.75f,  0.0f,  0.25f, 0.0f,1.0f,0.0f ,0.7f,// Top-right

    // Left face (X = 0.50f)
    0.50f, -0.25f, 0.25f,1.0f,0.0f,0.0f ,0.7f, // Bottom-back
    0.50f, -0.25f, 0.0f, 1.0f,0.0f,0.0f ,0.7f, // Bottom-front
    0.50f,  0.0f,  0.25f,1.0f,0.0f,0.0f ,0.7f, // Top-back
    0.50f, -0.25f, 0.0f,  1.0f,0.0f,0.0f ,0.7f, // Bottom-front
    0.50f,  0.0f,  0.25f, 1.0f,0.0f,0.0f ,0.7f, // Top-back
    0.50f,  0.0f,  0.0f,  1.0f,0.0f,0.0f ,0.7f,// Top-front

    // Right face (X = 0.75f)
    0.75f, -0.25f, 0.0f, 1.0f,0.0f,0.0f ,0.7f, // Bottom-front
    0.75f, -0.25f, 0.25f,1.0f,0.0f,0.0f ,0.7f, // Bottom-back
    0.75f,  0.0f,  0.0f,  1.0f,0.0f,0.0f ,0.7f,// Top-front
    0.75f, -0.25f, 0.25f, 1.0f,1.0f,1.0f ,0.7f, // Bottom-back
    0.75f,  0.0f,  0.0f, 1.0f,1.0f,0.0f ,0.7f, // Top-front
    0.75f,  0.0f,  0.25f,1.0f,0.0f,1.0f ,0.7f, // Top-back

    // Top face (Y = 0.0f)
    0.50f,  0.0f,  0.25f, 1.0f,0.0f,0.0f ,0.7f,// Back-left
    0.75f,  0.0f,  0.25f,1.0f,0.0f,0.0f ,0.7f, // Back-right
    0.50f,  0.0f,  0.0f, 1.0f,1.0f,0.0f ,0.7f, // Front-left
    0.75f,  0.0f,  0.25f,1.0f,1.0f,0.0f ,0.7f, // Back-right
    0.50f,  0.0f,  0.0f,  1.0f,1.0f,0.0f ,0.7f,// Front-left
    0.75f,  0.0f,  0.0f, 1.0f,1.0f,0.0f ,0.7f, // Front-right

    // Bottom face (Y = -0.25f)
    0.50f, -0.25f, 0.0f,  1.0f,0.0f,0.0f ,0.7f,// Front-left
    0.75f, -0.25f, 0.0f, 1.0f,0.0f,0.0f ,0.7f, // Front-right
    0.50f, -0.25f, 0.25f, 1.0f,0.0f,0.0f ,0.7f,// Back-left
    0.75f, -0.25f, 0.0f,  1.0f,0.0f,0.0f ,0.7f,// Front-right
    0.50f, -0.25f, 0.25f, 1.0f,0.0f,0.0f ,0.7f,// Back-left
    0.75f, -0.25f, 0.25f, 1.0f,0.0f,0.0f ,0.7f, // Back-right
	
	  -0.75f,	-0.75f,     0.0f, 1.0f,0.0f,0.0f ,0.7f,  //    (1)    |      /  (2)
	-0.25f ,   -0.75f,      0.0f, 1.0f,0.0f,0.0f ,0.7f,   //           |   /      
	  -0.75f,   -0.50f,       0.0f, 1.0f,0.0f,0.0f ,0.7f,  //       (3) |/         (4)
	 -0.25f ,   -0.75f,     0.0f, 1.0f,0.0f,0.0f ,0.7f,  // (2) 
	    -0.75f,   -0.50f,     0.0f, 1.0f,0.0f,0.0f ,0.7f,  // (3)
	  -0.25f , -0.50f,        0.0f, 1.0f,0.0f,0.0f ,0.7f,   // (4)
	  // rect 1
	  	-0.70f,	-0.72f,       0.1f, 1.0f,0.0f,0.0f ,0.7f,    // 1   (1)    |      /  (2)
		 -0.64f ,   -0.72f,   0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 2           |   /       
	  -0.70f,   -0.53f,       0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 3      (3) |/         (4)
	   -0.64f ,   -0.72f,     0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 4 (2) 
	    -0.70f,   -0.53f,     0.1f,  1.0f,0.0f,0.0f ,0.7f,  // 5 (3)
		  -0.64f , -0.53f,    0.0f,  1.0f,0.0f,0.0f ,0.7f,  // 6 (4)
	 // rect 2	  
		  
		  	  	-0.61f,	-0.72f,       0.1f,  1.0f,0.0f,0.0f ,0.7f,   // 1   (1)    |      /  (2)
		 -0.57f ,   -0.72f,   0.1f,  1.0f,0.0f,0.0f ,0.7f,   // 2           |   /       
	  -0.61f,   -0.53f,       0.1f,  1.0f,0.0f,0.0f ,0.7f,  // 3      (3) |/         (4)
	    -0.57f ,   -0.72f,     0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 4 (2) 
	    -0.61f,   -0.53f,     0.1f,  1.0f,0.0f,0.0f ,0.7f,  // 5 (3)
		  -0.57f , -0.53f,    0.0f,  1.0f,0.0f,0.0f ,0.7f,  // 6 (4)
		
 // rect 3
	  	 -0.54f,	-0.72f,       0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 1   (1)    |      /  (2)
	   	 -0.50f ,   -0.72f,   0.1f,  1.0f,0.0f,0.0f ,0.7f,  // 2           |   /       
	      -0.54f,   -0.53f,       0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 3      (3) |/         (4)
	      -0.50f ,   -0.72f,     0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 4 (2) 
	      -0.54f,   -0.53f,     0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 5 (3)
		  -0.50f , -0.53f,    0.0f,   1.0f,0.0f,0.0f ,0.7f, // 6 (4)
		  
		   // rect 4
	  	 -0.47f,	-0.72f,       0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 1   (1)    |      /  (2)
	   	 -0.41f ,   -0.72f,   0.1f,   1.0f,0.0f,0.0f ,0.7f,  // 2           |   /       
	      -0.47f,   -0.53f,       0.1f,  1.0f,0.0f,0.0f ,0.7f,   // 3      (3) |/         (4)
	      -0.41f ,   -0.72f,     0.1f,  1.0f,0.0f,0.0f ,0.7f,  // 4 (2) 
	      -0.47f,   -0.53f,     0.1f, 1.0f,0.0f,0.0f ,0.7f,   // 5 (3)
		  -0.41f , -0.53f,    0.0f,   1.0f,0.0f,0.0f ,0.7f, // 6 (4)
		  
		  
  };

//printf("size of triangles:%d",sizeof(vertices) ); ///(3*4));
//exit(0);

  // Create a VAO and VBO
  GLuint vao, vbo[1];
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, vbo);

  // Bind the VAO and VBO
  glBindVertexArray(vao);
  //glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	
  // Fill the VBO with the triangle vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Specify the vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);// Unbinds the VBO void VBO::Unbind()


   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
	 
	 
  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


// Simple perspective projection old
    float aspect = (float)width / height;
    float proj[16] = {
        1.0f/aspect, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.01f, -1.0f,
        0.0f, 0.0f, -2.01f, 0.0f
    };

    // Rotation matrix
   // angle += 0.01f;
    float c = cosf(angle);
    float s = sinf(angle);
    float rot[16] = {
        c, 0.0f, s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -s, 0.0f, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Translate cube away from camera
    float trans[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,   // (0)
		     0.0f, 1.0f, 0.0f, 0.0f,  // (1)
      //  0.0f, 1.0f, 0.25f, 0.0f, // (1)
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

    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvp);
//mvp

    // Draw the triangle
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 75);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Cleanup
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, vbo);
  glfwTerminate();

  return 0;
}