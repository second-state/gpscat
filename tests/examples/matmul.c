void matrixMul(int **A, int **B, int **C, int a, int b, int c) {
    for(int i = 0; i < a; ++i)
        for(int j = 0; j < c; ++j) {
            C[i][j] = 0;
            for(int k = 0; k < b; ++k)
                C[i][j] += A[i][k] * B[k][j];
        }
}
