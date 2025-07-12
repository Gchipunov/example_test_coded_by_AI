
// Generated and Fixed up By Georgiy Chipunov
// adding Destroy Terrian Feature

//----------------------------------------------------
// 4/20/2025
//Prompt Code in c opengl3 2d platformer a map system with spritemanager with destroyable terrian

// https://grok.com/chat/be438763-c808-4a1c-867f-334fdd64f86f 
//------------------------------------------------

// 3/25/2025
// build and link with
//compile with appropriate libraries:
//gcc tfbuild_text_renderer_grok.cpp -o tfbuild_text_renderer_grok -lopengl32 -lglew32 -lfreeglut -lglu32 -l ws2_32 -L"D:/games_src/example_test_coded_by_AI/freetype/lib" -lfreetype  -I "D:/games_src/example_test_coded_by_AI/freetype/include/freetype2" 
// g++ vao_multiple_text4.cpp grid_quad_generator.cpp engine/shader/shader.cpp   udpgame1/GameObjects.cpp -o vao_multiple_text4 -lopengl32 -lglew32 -lfreeglut -lglu32 -l ws2_32
// g++ stbgame_Box3.cpp UtilityCode/Utility.cpp grid_quad_generator.cpp audioengine.c udpgame1/GameObjects.cpp -o stbgame_Box3 -lopengl32 -lglew32 -lfreeglut -lglu32 -l ws2_32 -lalut -lpthread -L"D:/OpenAL_1-1_SDK/libs/Win64" -lOpenAL32 -I"D:/games_src/Box2D_v2.3.0/Box2D_v2.3.0/Box2D-install2/include" -L"D:/games_src/Box2D_v2.3.0/Box2D_v2.3.0/Box2D-install2/lib" -lBox2D 
// gcc tfbuild_text_renderer_grok2.cpp -o tfbuild_text_renderer_grok2 -lopengl32 -lglew32 -lfreeglut -lglu32 -l ws2_32 -L"D:/games_src/example_test_coded_by_AI/freetype/lib" -lfreetype  -I "D:/games_src/example_test_coded_by_AI/freetype/include/freetype2" 


/*
Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE) 
CMake Warning at builds/cmake/FindHarfBuzz.cmake:99 (message):
  Required version (2.0.0) is higher than found version ()
Call Stack (most recent call first):
  CMakeLists.txt:248 (find_package)


Could NOT find ZLIB (missing: ZLIB_LIBRARY ZLIB_INCLUDE_DIR) 
Could NOT find PNG (missing: PNG_LIBRARY PNG_PNG_INCLUDE_DIR) 
Could NOT find ZLIB (missing: ZLIB_LIBRARY ZLIB_INCLUDE_DIR) 
Could NOT find BZip2 (missing: BZIP2_LIBRARIES BZIP2_INCLUDE_DIR) 
Could NOT find BrotliDec (missing: BROTLIDEC_INCLUDE_DIRS BROTLIDEC_LIBRARIES) 
Configuring done (0.6s)
Generating done (0.1s)
*/
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <ft2build.h>
#include <freetype/freetype.h>
// -I "D:/games_src/example_test_coded_by_AI/freetype/include/freetype2"


// Shader structure

 struct Shader {

    GLuint program;

} ;



// Character structure

 struct Character {

    GLuint textureID;    // Texture ID for glyph

    int width, height;   // Size of glyph

    int bearingX, bearingY; // Offset from baseline

    unsigned int advance;   // Horizontal advance to next glyph

} ;



// Global variables

Character characters[128];

Shader textShader;

GLuint VAO, VBO;



const char* vertexShaderSource = 

    "#version 330 core\n"

    "layout (location = 0) in vec4 vertex;\n" // 

    "out vec2 TexCoords;\n"

    "uniform mat4 projection;\n"

    "void main()\n"

    "{\n"

    "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"

    "    TexCoords = vertex.zw;\n"

    "}\n";



const char* fragmentShaderSource = 

    "#version 330 core\n"

    "in vec2 TexCoords;\n"

    "out vec4 color;\n"

    "uniform sampler2D text;\n"

    "uniform vec3 textColor;\n"

    "void main()\n"

    "{\n"

    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"

    "    color = vec4(textColor, 1.0) * sampled;\n"

    "}\n";



// Function to compile shaders

Shader createShader() {

    Shader shader;

    

printf("createShader2\n");
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    glCompileShader(vertexShader);

    

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(fragmentShader);

    

    shader.program = glCreateProgram();

    glAttachShader(shader.program, vertexShader);

    glAttachShader(shader.program, fragmentShader);
 printf("glLinkProgram\n");
    glLinkProgram(shader.program);

    printf("glLinkProgram2\n");

    glDeleteShader(vertexShader);

    glDeleteShader(fragmentShader);

    

    return shader;

}



// Initialize FreeType and load characters

void initTextRenderer(const char* fontPath) {

    FT_Library ft;
printf("FT_Init_FreeType\n");
    if (FT_Init_FreeType(&ft)) {

        printf("ERROR: Could not init FreeType Library\n");

        return;

    }

printf("FT_New_Face\n");

    FT_Face face;

    if (FT_New_Face(ft, fontPath, 0, &face)) {

        printf("ERROR: Failed to load font\n");

        return;

    }


printf("FT_Set_Pixel_Sizes\n");
    FT_Set_Pixel_Sizes(face, 0, 48); // Set font size


printf("glPixelStorei\n");
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction



    // Load first 128 ASCII characters
printf("for FT_Load_Char\n");
    for (unsigned char c = 0; c < 128; c++) {

        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {

            printf("ERROR: Failed to load Glyph\n");

            continue;

        }



        GLuint texture;
printf("glGenTextures\n");
        glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(

            GL_TEXTURE_2D,

            0,

            GL_RED,

            face->glyph->bitmap.width,

            face->glyph->bitmap.rows,

            0,

            GL_RED,

            GL_UNSIGNED_BYTE,

            face->glyph->bitmap.buffer

        );

printf("glTexParameteri\n");

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


printf("Character character\n");
        Character character = {

            texture,

            face->glyph->bitmap.width,

            face->glyph->bitmap.rows,

            face->glyph->bitmap_left,

            face->glyph->bitmap_top,

            face->glyph->advance.x

        };

        characters[c] = character;

    }


printf("FT_Done_Face\n");
    FT_Done_Face(face);
printf("FT_Done_FreeType\n");
    FT_Done_FreeType(ft);



    // Configure VAO/VBO
printf("glGenVertexArrays\n");
    glGenVertexArrays(1, &VAO);
	
printf("glGenBuffers VBO\n");
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


printf("createShader\n");
    textShader = createShader();

}



// Render text function

void renderText(const char* text, float x, float y, float scale, float color[3], int screenWidth, int screenHeight) {

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    // Set up orthographic projection

    float projection[16] = {

        2.0f/screenWidth, 0.0f, 0.0f, 0.0f,

        0.0f, 2.0f/screenHeight, 0.0f, 0.0f,

        0.0f, 0.0f, -1.0f, 0.0f,

        -1.0f, -1.0f, 0.0f, 1.0f

    };



    glUseProgram(textShader.program);

    glUniform3f(glGetUniformLocation(textShader.program, "textColor"), color[0], color[1], color[2]);

    glUniformMatrix4fv(glGetUniformLocation(textShader.program, "projection"), 1, GL_FALSE, projection);

    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);



    // Iterate through all characters

    for (const char* c = text; *c; c++) {

        Character ch = characters[*c];



        float xpos = x + ch.bearingX * scale;

        float ypos = y - (ch.height - ch.bearingY) * scale;



        float w = ch.width * scale;

        float h = ch.height * scale;



        float vertices[6][4] = {

            { xpos,     ypos + h,   0.0f, 0.0f },

            { xpos,     ypos,       0.0f, 1.0f },

            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },

            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos + w, ypos + h,   1.0f, 0.0f }

        };



        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);



        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels

    }



    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);

}

void initGlut()
{
	  
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Grok ft2build fonts");

}


void render() {
	printf("color\n\n");
    float color[3] = {1.0f, 1.0f, 1.0f}; // White text
printf("while\n\n");
        renderText("Hello, OpenGL!", 100.0f, 100.0f, 1.0f, color, 800, 600);

	//    glBindVertexArray(0);

    glutSwapBuffers();
}
// Usage example
void idle() {
	
	glutPostRedisplay(); 
}

int main(int argc, char** argv) {

    // Initialize OpenGL context with GLEW here
 glutInit(&argc, argv);
initGlut();
printf("glewInit\n\n");
    glewInit();


printf("initTextRenderer\n\n");
    initTextRenderer("verdana.ttf");



   // while (1 ) {

        glClear(GL_COLOR_BUFFER_BIT);

        

        // Render text at position (100, 100) with scale 1.0

      //  renderText("Hello, OpenGL!", 100.0f, 100.0f, 1.0f, color, 800, 600);

        

        // Swap buffers and poll events

  // }
	glutIdleFunc(idle);

    glutDisplayFunc(render);
    glutMainLoop();



    return 0;

}