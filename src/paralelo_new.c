#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

#define MAX 1024
#define ITERACOES 1000
#define BORDA 127

typedef struct {
    unsigned char r, g, b;
    int fixo; // 1 se é ponto fixo
} Pixel;

int N;

// Função para alocar uma matriz de pixels
Pixel** alocaMatriz() {
    Pixel **mat = malloc(N * sizeof(Pixel*));
    for (int i = 0; i < N; i++)
        mat[i] = malloc(N * sizeof(Pixel));
    return mat;
}

// Função para copiar uma matriz para outra
void copiaMatriz(Pixel **dest, Pixel **orig) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            dest[i][j] = orig[i][j];
}

void liberaMatriz(Pixel **mat) {
    for (int i = 0; i < N; i++) free(mat[i]);
    free(mat);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s <entrada.txt> <saida.txt>\n", argv[0]);
        return 1;
    }

    FILE *fentrada = fopen(argv[1], "r");
    FILE *fsaida = fopen(argv[2], "w");
    if (!fentrada || !fsaida) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    int numFixos;
    fscanf(fentrada, "%d %d", &N, &numFixos);

    Pixel **matA = alocaMatriz();
    Pixel **matB = alocaMatriz();

    // Inicializa todas as posições
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matA[i][j].r = matA[i][j].g = matA[i][j].b = 0;
            matA[i][j].fixo = 0;
            if (i == 0 || i == N-1 || j == 0 || j == N-1) {
                matA[i][j].r = BORDA;
                matA[i][j].g = BORDA;
                matA[i][j].b = BORDA;
            }
        }
    }

    copiaMatriz(matB, matA);

    // Lê pontos fixos
    for (int i = 0; i < numFixos; i++) {
        int x, y;
        int r, g, b;
        fscanf(fentrada, "%d %d %d %d %d", &x, &y, &r, &g, &b);
        matA[x][y].r = r;
        matA[x][y].g = g;
        matA[x][y].b = b;
        matA[x][y].fixo = 1;
    }
    copiaMatriz(matB, matA);

    double inicio = omp_get_wtime();

    // Laço principal de iteração
    for (int it = 0; it < ITERACOES; it++) {
        #pragma omp parallel for collapse(2)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < N-1; j++) {
                if (matA[i][j].fixo) continue;

                matB[i][j].r = (matA[i][j].r + matA[i-1][j].r + matA[i+1][j].r + matA[i][j-1].r + matA[i][j+1].r) / 5;
                matB[i][j].g = (matA[i][j].g + matA[i-1][j].g + matA[i+1][j].g + matA[i][j-1].g + matA[i][j+1].g) / 5;
                matB[i][j].b = (matA[i][j].b + matA[i-1][j].b + matA[i+1][j].b + matA[i][j-1].b + matA[i][j+1].b) / 5;
            }
        }

        // Troca os ponteiros (ping-pong)
        Pixel **tmp = matA;
        matA = matB;
        matB = tmp;
    }

    double fim = omp_get_wtime();
    printf("Tempo: %.5f segundos\n", fim - inicio);

    // Escreve a saída (colunas 65 a 128)
    for (int i = 0; i < N; i++) {
        for (int j = 64; j < 128; j++) {
            fprintf(fsaida, "%d %d %d ", matA[i][j].r, matA[i][j].g, matA[i][j].b);
        }
        fprintf(fsaida, "\n");
    }

    liberaMatriz(matA);
    liberaMatriz(matB);
    fclose(fentrada);
    fclose(fsaida);

    return 0;
}
