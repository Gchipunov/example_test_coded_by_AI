#include <stdio.h>
#include <math.h>

// Function to multiply two 3x3 matrices
void matrixMultiply(float mat1[3][3], float mat2[3][3], float result[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                result[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
}

// Function to transpose a 3x3 matrix
void transposeMatrix(float mat[3][3], float result[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result[j][i] = mat[i][j];
        }
    }
}

// Function to print a 3x3 matrix
void printMatrix(float mat[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%6.3f ", mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    // Create a 3x3 rotation matrix around Z-axis (orthogonal matrix)
    float angle = 45.0f * M_PI / 180.0f; // 45 degrees in radians
    float orthoMatrix[3][3] = {
        {cosf(angle), -sinf(angle), 0.0f},
        {sinf(angle),  cosf(angle), 0.0f},
        {0.0f,        0.0f,        1.0f}
    };

    // Variables for verification
    float transpose[3][3];
    float result[3][3];

    printf("Original Orthogonal Matrix (Rotation by 45 degrees):\n");
    printMatrix(orthoMatrix);

    // Get transpose
    transposeMatrix(orthoMatrix, transpose);
    printf("Transpose of the Matrix:\n");
    printMatrix(transpose);

    // Multiply matrix with its transpose to verify orthogonality
    // For an orthogonal matrix, A * A^T = I (identity matrix)
    matrixMultiply(orthoMatrix, transpose, result);
    printf("Matrix * Transpose (should be identity matrix):\n");
    printMatrix(result);

    // Check if result is approximately identity matrix
    float identity[3][3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };
    
    float tolerance = 0.0001f;
    int isOrthogonal = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (fabs(result[i][j] - identity[i][j]) > tolerance) {
                isOrthogonal = 0;
                break;
            }
        }
    }

    printf("Is matrix orthogonal? %s\n", isOrthogonal ? "Yes" : "No");

    return 0;
}