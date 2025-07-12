
// Generated and Fixed up By Georgiy Chipunov
// 3/25/2025
// build and link with
//compile with appropriate libraries:

// gcc stb_freetype_text_renderer_grok.cpp -o stb_freetype_text_renderer_grok -lGL -lGLEW -lfreetype

// g++ stb_freetype_text_renderer_grok.cpp -o stb_freetype_text_renderer_grok -lGL -lGLEW -lfreetype

// gcc stb_freetype_text_renderer_grok.cpp -o stb_freetype_text_renderer_grok -lopengl32 -lglew32 -lfreeglut -lglu32 -l ws2_32 -lstdc++

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

// #include <cstdio>

//#include <iostream>

#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_truetype.h"

// Shader structure

 struct Shader {

    GLuint program;

} ;



// Character structure

 struct  Character {

    GLuint textureID;    // Texture ID for glyph

    int width, height;   // Size of glyph

    int bearingX, bearingY; // Offset from baseline

    float advance;       // Horizontal advance to next glyph

} ;



// Global variables

Character characters[128];

Shader textShader;

GLuint VAO, VBO;

unsigned char* ttf_buffer;

stbtt_fontinfo font;



const char* vertexShaderSource = 

    "#version 330 core\n"

    "layout (location = 0) in vec4 vertex;\n"

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



// Compile shaders

Shader createShader() {

    Shader shader;

    

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    glCompileShader(vertexShader);

    

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    glCompileShader(fragmentShader);

    

    shader.program = glCreateProgram();

    glAttachShader(shader.program, vertexShader);

    glAttachShader(shader.program, fragmentShader);

    glLinkProgram(shader.program);

    

    glDeleteShader(vertexShader);

    glDeleteShader(fragmentShader);

    

    return shader;

}



// Initialize text renderer

void initTextRenderer(const char* fontPath, float fontSize) {

    // Load font file

    FILE* fp = fopen(fontPath, "rb");

    if (!fp) {

        printf("ERROR: Could not open font file\n");

        return;

    }

    

    fseek(fp, 0, SEEK_END);

    long size = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    

    ttf_buffer = (unsigned char*)malloc(size);

    fread(ttf_buffer, 1, size, fp);

    fclose(fp);



    if (!stbtt_InitFont(&font, ttf_buffer, 0)) {

        printf("ERROR: Failed to initialize font\n");

        free(ttf_buffer);

        return;

    }



    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);



    // Load ASCII characters

    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

    for (unsigned char c = 32; c < 128; c++) {

        int width, height, xoff, yoff;

        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, c, 

                                                       &width, &height, &xoff, &yoff);



        GLuint texture;

        glGenTextures(1, &texture);

        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, 

                    GL_RED, GL_UNSIGNED_BYTE, bitmap);



        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



        int advance;

        stbtt_GetCodepointHMetrics(&font, c, &advance, NULL);



        Character character = {

            texture,

            width,

            height,

            xoff,

            yoff,

            advance * scale

        };

        characters[c] = character;



        stbtt_FreeBitmap(bitmap, NULL);

    }



    // Configure VAO/VBO

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);



    textShader = createShader();

}



// Render text

void renderText(const char* text, float x, float y, float scale, float color[3], int screenWidth, int screenHeight) {

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



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



    for (const char* c = text; *c; c++) {

        if (*c < 32 || *c >= 128) continue;

        

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



        x += ch.advance * scale;

    }



    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);

}



// Cleanup function

void cleanupTextRenderer() {

    for (int i = 32; i < 128; i++) {

        glDeleteTextures(1, &characters[i].textureID);

    }

    glDeleteBuffers(1, &VBO);

    glDeleteVertexArrays(1, &VAO);

    glDeleteProgram(textShader.program);

    free(ttf_buffer);

}



// Usage example

int main() {

    // Initialize OpenGL context with GLEW

    glewInit();



    initTextRenderer("verdana.ttf", 48.0f);



    float color[3] = {1.0f, 1.0f, 1.0f}; // White text

    while (1) {

        glClear(GL_COLOR_BUFFER_BIT);

        

        renderText("Hello, OpenGL!", 100.0f, 100.0f, 1.0f, color, 800, 600);

        

        // Swap buffers and poll events

    }



    cleanupTextRenderer();

    return 0;

}