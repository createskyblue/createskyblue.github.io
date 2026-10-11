/*
 * branch_demo.c - loxseq 运行中分支示例
 *
 * 流程：灌装 -> 质检 -> 打包。
 * 质检按业务结果分支：合格跳过重工直接打包，不合格先跳去重工。
 * 编译运行：gcc -I../include branch_demo.c ../src/loxseq.c
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "loxseq/loxseq.h"

/* 4 个步骤：灌装 -> 质检 -> 打包；质检不合格时先跳去重工 */
enum { S_FILL, S_INSPECT, S_REWORK, S_PACKAGE, STEP_COUNT };

static int g_pass;   /* 用户标志：质检结果，1=合格 0=不合格 */

/* 灌装：跑满 1000ms 算完成 */
static loxseq_step_status_t step_fill(loxseq_t *s, uint32_t now, void *u) {
    (void)u; (void)s;
    if (now >= 1000) return LOXSEQ_STEP_DONE;
    return LOXSEQ_STEP_RUNNING;
}

/* 质检：做完后按结果分支 */
static loxseq_step_status_t step_inspect(loxseq_t *s, uint32_t now, void *u) {
    (void)u;
    if (now < 2000) return LOXSEQ_STEP_RUNNING;  /* 还没检完，下次 tick 再来 */
    if (g_pass) {
        loxseq_set_next_step(s, S_PACKAGE);     /* 合格：跳过重工，直接打包 */
    } else {
        loxseq_set_next_step(s, S_REWORK);      /* 不合格：先跳去重工 */
    }
    return LOXSEQ_STEP_BRANCH;                  /* 告诉序列器：这次要跳 */
}

/* 重工：只有不合格分支会走到 */
static loxseq_step_status_t step_rework(loxseq_t *s, uint32_t now, void *u) {
    (void)u; (void)s;
    if (now >= 3000) return LOXSEQ_STEP_DONE;
    return LOXSEQ_STEP_RUNNING;
}

/* 打包：流程终点，完成后整条序列结束 */
static loxseq_step_status_t step_package(loxseq_t *s, uint32_t now, void *u) {
    (void)u; (void)s;
    if (now >= 1000) return LOXSEQ_STEP_DONE;
    return LOXSEQ_STEP_RUNNING;
}

/* 步骤表，一行一步：标签 + 动作 + 超时 + 断电恢复策略 */
static const loxseq_step_def_t steps[STEP_COUNT] = {
    [S_FILL]    = { .tag = "fill",    .action = step_fill,
                    .timeout_ms = 10000, .resume_policy = LOXSEQ_RESUME_AT_STEP },
    [S_INSPECT] = { .tag = "inspect", .action = step_inspect,
                    .timeout_ms = 10000, .resume_policy = LOXSEQ_RESUME_AT_STEP },
    [S_REWORK]  = { .tag = "rework",  .action = step_rework,
                    .timeout_ms = 10000, .resume_policy = LOXSEQ_RESUME_AT_STEP },
    [S_PACKAGE] = { .tag = "package", .action = step_package,
                    .timeout_ms = 10000, .resume_policy = LOXSEQ_RESUME_AT_STEP },
};

/* --- 存储与安全初始化：测试用 RAM，真实场景换成 Flash --- */

static uint8_t g_buf[LOXSEQ_RECORD_SERIALIZED_SIZE];
static size_t g_len;

static int ram_write(const void *b, size_t n, void *u) {
    (void)u; if (n != sizeof(g_buf)) return -1;
    memcpy(g_buf, b, n); g_len = n; return 0;
}
static int ram_read(void *b, size_t n, void *u) {
    (void)u; if (n != g_len) return -1;
    memcpy(b, g_buf, n); return 0;
}
static int ram_erase(void *u) { (void)u; g_len = 0; return 0; }

static loxseq_err_t safe_init(loxseq_t *s, uint32_t t, void *u) {
    (void)s; (void)t; (void)u; return LOXSEQ_OK;
}

/* --- 跑一遍流程，打印实际走过的步骤顺序 --- */

static void run_case(const char *name, int pass) {
    static const loxseq_storage_t storage = {
        .write_checkpoint = ram_write, .read_checkpoint = ram_read, .erase_checkpoint = ram_erase,
    };
    static const loxseq_safe_init_t safe_cfg = { .run = safe_init, .user = NULL };

    g_pass = pass;
    loxseq_t seq;
    loxseq_init(&seq, steps, STEP_COUNT, &storage, &safe_cfg);
    loxseq_recover(&seq, LOXSEQ_REBOOT_NORMAL);
    loxseq_start_fresh(&seq, 0);

    printf("[%s] 步骤顺序: ", name);
    const char *prev = "";
    for (uint32_t t = 0; t < 50000; t += 100) {
        loxseq_tick(&seq, t);
        const char *tag = loxseq_current_tag(&seq);
        if (tag && strcmp(tag, prev) != 0) { printf("%s ", tag); prev = tag; }
        if (loxseq_state(&seq) == LOXSEQ_STATE_COMPLETE) { printf("-> 完成\n"); break; }
        if (loxseq_state(&seq) == LOXSEQ_STATE_FAILED)   { printf("-> FAILED\n"); break; }
    }
}

int main(void) {
    run_case("合格(跳过重工)", 1);
    run_case("不合格(先去重工)", 0);
    return 0;
}
