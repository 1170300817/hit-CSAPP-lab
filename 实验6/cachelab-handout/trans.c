
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int x0,x1,x2,x3,x4,x5,x6,x7;
    if (N == 32)
    {
        for (int i = 0; i < N; i += 8)
        {
            for (int j = 0; j < M; j += 8)
            {
                for (int x = i; x < i + 8; ++x)
                {
                    if (i==j)
                    {
                        x0 = A[x][j];
                        x1 = A[x][j+1];
                        x2 = A[x][j+2];
                        x3 = A[x][j+3];
                        x4 = A[x][j+4];
                        x5 = A[x][j+5];
                        x6 = A[x][j+6];
                        x7 = A[x][j+7];
                        B[j][x] = x0;
                        B[j+1][x] = x1;
                        B[j+2][x] = x2;
                        B[j+3][x] = x3;
                        B[j+4][x] = x4;
                        B[j+5][x] = x5;
                        B[j+6][x] = x6;
                        B[j+7][x] = x7;
                    }
                    else
                    {
                        for (int y = j; y< j + 8; ++y)
                        {
                            B[y][x]= A[x][y];
                        }
                    }
                }
            }
        }
    }
    else if (N == 64)
    {
        for (int i = 0; i < N; i += 8)
        {
            for (int j = 0; j < M; j += 8)
            {
                for (int x = i; x < i + 4; ++x)
                {
                    for(x=i; x<i+4; x++)
                    {
                        x0 = A[x][j];
                        x1 = A[x][j+1];
                        x2 = A[x][j+2];
                        x3 = A[x][j+3];
                        x4 = A[x][j+4];
                        x5 = A[x][j+5];
                        x6 = A[x][j+6];
                        x7 = A[x][j+7];
                        B[j][x] = x0;
                        B[j+1][x] = x1;
                        B[j+2][x] = x2;
                        B[j+3][x] = x3;
                        B[j][x+4] = x4;
                        B[j+1][x+4] = x5;
                        B[j+2][x+4] = x6;
                        B[j+3][x+4] = x7;
                    }
                    for(int y=j; y<j+4; y++)
                    {
                        x0 = A[i+4][y];
                        x1 = A[i+5][y];
                        x2 = A[i+6][y];
                        x3 = A[i+7][y];
                        x4 = B[y][i+4];
                        x5 = B[y][i+5];
                        x6 = B[y][i+6];
                        x7 = B[y][i+7];
                        B[y][i+4] = x0;
                        B[y][i+5] = x1;
                        B[y][i+6] = x2;
                        B[y][i+7] = x3;
                        B[y+4][i]= x4;
                        B[y+4][i+1] = x5;
                        B[y+4][i+2] = x6;
                        B[y+4][i+3] = x7;
                    }
                    for (x=i+4; x<i+8; x++)
                    {
                        x0 = A[x][j+4];
                        x1 = A[x][j+5];
                        x2 = A[x][j+6];
                        x3 = A[x][j+7];
                        B[j+4][x] = x0;
                        B[j+5][x] = x1;
                        B[j+6][x] = x2;
                        B[j+7][x] = x3;

                    }

                }
            }
        }
    }
    else
    {
        for(int i=0; i<N; i+=16)
        {
            for(int j=0; j<M; j+=16)
            {
                for(int x=i; x<N&&x<i+16; x++)
                {
                    for(int y=j; y<M&&y<j+16; y++)
                    {
                        B[y][x]=A[x][y];
                    }
                }
            }
        }
    }
}


void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);
}
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}

