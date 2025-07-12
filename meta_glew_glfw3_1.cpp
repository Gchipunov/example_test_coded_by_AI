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

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main(int argc, char** argv) {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Create a windowed mode window and its OpenGL context
  GLFWwindow* window = glfwCreateWindow(640, 480, "Triangle Example", NULL, NULL);
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

  // Define the triangle vertices
  GLfloat vertices[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.0f,  0.5f
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
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(0);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the triangle
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

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