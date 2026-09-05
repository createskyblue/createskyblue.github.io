#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcheck.h>

char *create_packet(int n) {
    char *p = malloc(1024 + n);
    if (!p) return NULL;
    memset(p, 0x41, 1024 + n);
    return p;
}

void lost_pointer(void) {
    char *p = malloc(512);
    if (!p) return;
    p = malloc(256);
    if (!p) return;
    free(p);
}

int main(int argc, char **argv) {
    mtrace();   /* 开启记录 */
    int iters = 100;
    if (argc > 1) iters = atoi(argv[1]);
    for (int i = 0; i < iters; i++) {
        char *buf = create_packet(i);
        lost_pointer();
    }
    printf("done\n");
    muntrace(); /* 关闭并落盘 */
    return 0;
}
