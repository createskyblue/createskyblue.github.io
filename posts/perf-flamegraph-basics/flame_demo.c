/*
 * flame_demo.c — 性能分析演示程序
 *
 * 设计目标：让火焰图"好读"。
 *   - main -> run_all -> 4 个部分：矩阵乘 / 快排 / 哈希 / 斐波那契
 *   - 热点明显：矩阵乘是最大的宽块
 *   - 递归：斐波那契在图上形成高塔
 *
 * 用法：
 *   ./flame_demo [每部分秒数]           # 默认 5 秒/部分，共 4 部分
 *
 * 自采样模式（模拟 perf 的采样原理，编译时加 -DSELF_SAMPLE）：
 *   gcc -O2 -g -rdynamic -fno-omit-frame-pointer -DSELF_SAMPLE -o flame_demo_ss flame_demo.c
 *   ./flame_demo_ss 4 > folded.txt      # 输出 folded 格式 -> 喂给 flamegraph.pl
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#ifdef SELF_SAMPLE
#include <signal.h>
#include <sys/time.h>
#include <execinfo.h>

#define MAX_FRAMES  64
#define MAX_SAMPLES 60000
static void *s_stack[MAX_SAMPLES][MAX_FRAMES];
static int   s_nframes[MAX_SAMPLES];
static volatile sig_atomic_t s_count;

static void sample_handler(int sig)
{
    int i = s_count;
    if (i >= MAX_SAMPLES) return;
    s_nframes[i] = backtrace(s_stack[i], MAX_FRAMES);
    __sync_synchronize();
    s_count = i + 1;
}

static int skip_frame(const char *name)
{
    return strstr(name, "sample_handler") || strstr(name, "__restore_rt")
        || strstr(name, "__kernel_rt_sigreturn") || strstr(name, "backtrace");
}

static void dump_folded(void)
{
    int n = s_count;
    for (int i = 0; i < n; i++) {
        char **names = backtrace_symbols(s_stack[i], s_nframes[i]);
        if (!names) continue;
        fputs("flame_demo", stdout);           /* 根 */
        for (int f = s_nframes[i] - 1; f >= 0; f--) {
            char *p = strchr(names[f], '(');
            char *q = p ? strchr(p, '+') : NULL;
            if (!p || !q) continue;
            *q = '\0';
            if (skip_frame(p + 1)) continue;
            printf(";%s", p + 1);
        }
        printf(" 1\n");
        free(names);
    }
}
#endif

static double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ================= 1. 矩阵乘法：O(n^3)，全程序最大热点 ================= */
static void matmul(int n, double *restrict c,
                   const double *restrict a, const double *restrict b)
{
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++) {
            const double aik = a[i * n + k];
            for (int j = 0; j < n; j++)
                c[i * n + j] += aik * b[k * n + j];
        }
}

static void part_matmul(double budget)
{
    enum { N = 512 };
    static double a[N * N], b[N * N], c[N * N];
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            a[i * N + j] = (i * 31 + j * 17) * 0.001;
            b[i * N + j] = (i * 7 - j * 13) * 0.001;
        }
    double t0 = now_sec();
    int it = 0;
    do {
        memset(c, 0, sizeof c);
        matmul(N, c, a, b);
        it++;
    } while (now_sec() - t0 < budget);
    fprintf(stderr, "part_matmul : %2d 次, c[0]=%.3f\n", it, c[0]);
}

/* ================= 2. 快速排序：递归 + 大量内存访问 ================= */
static void qsort_impl(int *v, int lo, int hi)
{
    if (lo >= hi) return;
    int pivot = v[(lo + hi) >> 1], i = lo, j = hi;
    while (i <= j) {
        while (v[i] < pivot) i++;
        while (v[j] > pivot) j--;
        if (i <= j) {
            int t = v[i]; v[i] = v[j]; v[j] = t;
            i++; j--;
        }
    }
    qsort_impl(v, lo, j);
    qsort_impl(v, i, hi);
}

static void quicksort(int *v, int n) { qsort_impl(v, 0, n - 1); }

static void part_qsort(double budget)
{
    enum { N = 2000000 };
    static int v[N];
    double t0 = now_sec();
    int it = 0;
    do {
        for (int i = 0; i < N; i++)
            v[i] = (int)((i * 2654435761u) ^ (i >> 3));
        quicksort(v, N);
        it++;
    } while (now_sec() - t0 < budget);
    fprintf(stderr, "part_qsort  : %2d 次, v[N/2]=%d\n", it, v[N / 2]);
}

/* ================= 3. 字符串哈希：模拟文本/日志处理 ================= */
static uint32_t hash_stream(const uint8_t *buf, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    for (size_t i = 0; i < len; i++) {
        h ^= buf[i];
        h *= 0x5bd1e995u;
        h ^= h >> 15;
    }
    return h;
}

static void part_hash(double budget)
{
    enum { N = 1 << 20 };
    static uint8_t buf[N];
    for (size_t i = 0; i < N; i++) buf[i] = (uint8_t)(i * 2654435761u);
    double t0 = now_sec();
    uint32_t h = 0;
    size_t total = 0;
    do {
        h ^= hash_stream(buf, N, h);
        total += N;
    } while (now_sec() - t0 < budget);
    fprintf(stderr, "part_hash   : %4zu MB, h=%08x\n", total >> 20, h);
}

/* ================= 4. 斐波那契：深度递归，火焰图上的"塔" ================= */
static uint64_t fib(int n)
{
    if (n <= 1) return (uint64_t)n;
    return fib(n - 1) + fib(n - 2);
}

static void part_fib(double budget)
{
    enum { N = 36 };
    double t0 = now_sec();
    uint64_t s = 0;
    int it = 0;
    do {
        s += fib(N);
        it++;
    } while (now_sec() - t0 < budget);
    fprintf(stderr, "part_fib    : %2d 次, s=%llu\n", it, (unsigned long long)s);
}

/* ================= 调度 ================= */
static void run_all(double budget)
{
    part_matmul(budget);
    part_qsort(budget);
    part_hash(budget);
    part_fib(budget);
}

int main(int argc, char **argv)
{
    double budget = 5.0;
    if (argc > 1) budget = atof(argv[1]);
    if (budget < 0.5) budget = 0.5;

#ifdef SELF_SAMPLE
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = sample_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPROF, &sa, NULL);

    struct itimerval it;
    memset(&it, 0, sizeof it);
    it.it_interval.tv_usec = 10000;   /* 10ms -> 100Hz */
    it.it_value.tv_usec = 10000;
    setitimer(ITIMER_PROF, &it, NULL);
    fprintf(stderr, "# self-sample mode: 100 Hz\n");
#endif

    fprintf(stderr, "flame_demo: budget=%.1fs/part, pid=%d\n", budget, (int)getpid());
    run_all(budget);
    fprintf(stderr, "done.\n");

#ifdef SELF_SAMPLE
    struct itimerval zero;
    memset(&zero, 0, sizeof zero);
    setitimer(ITIMER_PROF, &zero, NULL);
    fprintf(stderr, "# samples: %d\n", (int)s_count);
    dump_folded();
#endif
    return 0;
}
