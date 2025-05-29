/* Programa Paralelo */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#define MAX_N 1024

typedef struct {
    unsigned char r, g, b;
    int is_fixed;
} Pixel;

Pixel **alloc_matrix(int n) {
    Pixel **mat = malloc(n * sizeof(Pixel *));
    for (int i = 0; i < n; i++) {
        mat[i] = malloc(n * sizeof(Pixel));
    }
    return mat;
}

void free_matrix(Pixel **mat, int n) {
    for (int i = 0; i < n; i++) {
        free(mat[i]);
    }
    free(mat);
}

void copy_matrix(Pixel **dest, Pixel **src, int n) {
    for (int i = 0; i < n; i++) {
        memcpy(dest[i], src[i], n * sizeof(Pixel));
    }
}

void initialize_matrix(Pixel **mat, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == 0 || j == 0 || i == n-1 || j == n-1) {
                mat[i][j].r = 127;
                mat[i][j].g = 127;
                mat[i][j].b = 127;
            } else {
                mat[i][j].r = 0;
                mat[i][j].g = 0;
                mat[i][j].b = 0;
            }
            mat[i][j].is_fixed = 0;
        }
    }
}

void read_input(const char *filename, Pixel **mat, int *n) {
    FILE *f = fopen(filename, "r");
    int num_fixed;
    fscanf(f, "%d %d", n, &num_fixed);
    initialize_matrix(mat, *n);
    for (int i = 0; i < num_fixed; i++) {
        int x, y;
        int r, g, b;
        fscanf(f, "%d %d %d %d %d", &x, &y, &r, &g, &b);
        mat[x][y].r = r;
        mat[x][y].g = g;
        mat[x][y].b = b;
        mat[x][y].is_fixed = 1;
    }
    fclose(f);
}

void write_output(Pixel **mat, int n, const char *filename) {
    FILE *f = fopen(filename, "w");
    for (int i = 0; i < n; i++) {
        for (int j = 64; j < 128; j++) {
            fprintf(f, "%d %d %d ", mat[i][j].r, mat[i][j].g, mat[i][j].b);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void stencil_openmp(Pixel **current, Pixel **next, int n, int iter, int num_threads) {
    double start = omp_get_wtime();

    for (int it = 0; it < iter; it++) {
        #pragma omp parallel for collapse(2) num_threads(num_threads)
        for (int i = 1; i < n-1; i++) {
            for (int j = 1; j < n-1; j++) {
                if (current[i][j].is_fixed) {
                    next[i][j] = current[i][j];
                    continue;
                }

                int r = current[i][j].r + current[i-1][j].r + current[i+1][j].r +
                        current[i][j-1].r + current[i][j+1].r;
                int g = current[i][j].g + current[i-1][j].g + current[i+1][j].g +
                        current[i][j-1].g + current[i][j+1].g;
                int b = current[i][j].b + current[i-1][j].b + current[i+1][j].b +
                        current[i][j-1].b + current[i][j+1].b;

                next[i][j].r = r / 5;
                next[i][j].g = g / 5;
                next[i][j].b = b / 5;
                next[i][j].is_fixed = 0;
            }
        }

        // Troca os ponteiros
        Pixel **tmp = current;
        current = next;
        next = tmp;
    }

    double end = omp_get_wtime();
    printf("Tempo com %d threads: %.6f segundos\n", num_threads, end - start);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <entrada.txt> <saida.txt> <num_threads>\n", argv[0]);
        return 1;
    }

    int n;
    Pixel **mat1 = alloc_matrix(MAX_N);
    Pixel **mat2 = alloc_matrix(MAX_N);

    read_input(argv[1], mat1, &n);

    stencil_openmp(mat1, mat2, n, 1000, atoi(argv[3]));

    write_output(mat1, n, argv[2]);

    free_matrix(mat1, n);
    free_matrix(mat2, n);

    return 0;
}

// Compile with: gcc -fopenmp paralelo.c -o paralelo
// Run with: ./paralelo entrada.txt saida.txt 4
// Note: Ensure OpenMP is supported by your compiler and linked correctly.
// Make sure to adjust the number of threads as needed for your system.
// The input file should be formatted as follows:
// <n> <num_fixed>
// <x1> <y1> <r1> <g1> <b1>
// <x2> <y2> <r2> <g2> <b2>
// ...  