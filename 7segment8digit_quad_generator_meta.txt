#include <GLFW/glfw3.h>
#include <stdio.h>

// Define the vertices for the quad
GLfloat quadVertices[] = {
    -0.5f, -0.5f, 0.0f, // Bottom left
     0.5f, -0.5f, 0.0f, // Bottom right
     0.5f,  0.5f, 0.0f, // Top right
    -0.5f,  0.5f, 0.0f  // Top left
};

// Define the vertices for a single 7-segment digit
GLfloat digitVertices[] = {
    // Top horizontal segment
    -0.05f,  0.15f, 0.0f,
     0.05f,  0.15f, 0.0f,

    // Top-left vertical segment
    -0.05f,  0.15f, 0.0f,
    -0.05f,  0.05f, 0.0f,

    // Top-right vertical segment
     0.05f,  0.15f, 0.0f,
     0.05f,  0.05f, 0.0f,

    // Middle horizontal segment
    -0.05f,  0.05f, 0.0f,
     0.05f,  0.05f, 0.0f,

    // Bottom-left vertical segment
    -0.05f,  0.05f, 0.0f,
    -0.05f, -0.05f, 0.0f,

    // Bottom-right vertical segment
     0.05f,  0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,

    // Bottom horizontal segment
    -0.05f, -0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,
};

// Function to draw a single 7-segment digit
void drawDigit(GLfloat x, GLfloat y, int digit) {
    // Define the segments to draw for each digit
    int segments[][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}  // 9
    };

    // Draw the segments for the current digit
    for (int i = 0; i < 7; i++) {
        if (segments[digit][i]) {
            // Define the vertices for the current segment
            GLfloat segmentVertices[6];
            switch (i) {
                case 0: // Top horizontal segment
                    segmentVertices[0] = x - 0.05f; segmentVertices[1] = y + 0.15f; segmentVertices[2] = 0.0f;
                    segmentVertices[3] = x + 0.05f; segmentVertices[4] = y + 0.15f; segmentVertices[5] = 0.0f;
                    break;
                case 1: // Top-left vertical segment
                    segmentVertices[0] = x - 0.05f; segmentVertices[1] = y + 0.15f; segmentVertices[2] = 0.0f;
                    segmentVertices3