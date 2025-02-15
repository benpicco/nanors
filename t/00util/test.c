#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "rs.h"

#define MAP(x, max_x, m, n) (m + x / (max_x / (n - m) + 1))

typedef reed_solomon rs_t;

int run(int seed, int K, int N, int T)
{
    uint8_t **buf = calloc(2 * K, T * sizeof(uint8_t *));
    uint8_t **cmp = calloc(2 * K, T * sizeof(uint8_t *));
    uint8_t *marks = calloc(1, 2 * K);
    int ret = 0;

    for (int i = 0; i < 2 * K; i++) {
        buf[i] = calloc(1, T);
        cmp[i] = calloc(1, T);
    }

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < T; j++) {
            buf[i][j] = MAP(rand(), RAND_MAX, 0, 256);
            cmp[i][j] = buf[i][j];
        }
    }

    reed_solomon_init();
    rs_t *rs = reed_solomon_new(K, N);
    if (!rs) {
        free(buf);
        free(cmp);
        free(marks);
        return -1;
    }
    reed_solomon_encode(rs, buf, K + N, T);

    for (int i = 0; i < K + N; i++) {
        marks[i] = 0;
    }

    for (int i = 0; i < N; i++) {
        int at = rand() % (K + N);
        memset(buf[at], 0, T);
        marks[at] = 1;
    }

    ret = reed_solomon_reconstruct(rs, buf, marks, K + N, T);
    reed_solomon_release(rs);

    int failed = 0;
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < T; j++) {
            if (cmp[i][j] != buf[i][j]) {
                printf("mismatch at row %d col %d\n", i, j);
                failed = 1;
                break;
            }
        }
    }

    printf("===%s===P SEED: %d K: %d N: %d T: %d\n", failed ? "FAILED" : "OK", seed, K, N, T);
    for (int i = 0; i < 2 * K; i++) {
        free(buf[i]);
        free(cmp[i]);
    }

    free(buf);
    free(cmp);
    free(marks);
    return ret;
}

int main(int argc, char *argv[])
{
    int seed = time(NULL), K, N, T;
    if (argc == 2) {
        seed = strtol(argv[1], NULL, 10);
    }
    srand(seed);

    K = MAP(rand(), RAND_MAX, 4, 256);
    N = MAP(rand(), RAND_MAX, 4, (K / 4) + 4);
    if ((K + N) > 255)
        K -= N;
    T = MAP(rand(), RAND_MAX, 200, 1460);

    printf("===BEGIN===P SEED: %d K: %d N: %d T: %d\n", seed, K, N, T);

    int num = 1;
    for (int i = 0; i < num; i++) {
        run(seed, K, N, T);
    }
    return 0;
}
