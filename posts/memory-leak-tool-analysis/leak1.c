#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 场景1: 典型的"忘了 free" —— 每次调用泄漏一块堆内存 */
char *create_packet(int n) {
    char *p = malloc(1024 + n);
    if (!p) return NULL;
    memset(p, 0x41, 1024 + n);
    return p;   /* 调用方忘记 free */
}

/* 场景2: 指针被覆盖 —— 先 malloc 没 free 就重新赋值，旧块直接丢了 */
void lost_pointer(void) {
    char *p = malloc(512);
    if (!p) return;
    p = malloc(256);   /* 512 字节那块永远找不回来了 */
    if (!p) return;
    free(p);
}

int main(int argc, char **argv) {
    int iters = 100;
    if (argc > 1) iters = atoi(argv[1]);
    for (int i = 0; i < iters; i++) {
        char *buf = create_packet(i);
        /* 故意不 free(buf) */
        lost_pointer();
        if (i % 10 == 0) printf("iter %d ...\n", i);
    }
    printf("done %d iters\n", iters);
    return 0;
}
