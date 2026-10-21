#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t ror32(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32u - n));
}

static uint32_t rol32(uint32_t x, unsigned n) {
    return (x << n) | (x >> (32u - n));
}


int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t lo  = (uint32_t)challenge;
    uint32_t hi  = (uint32_t)(challenge >> 32);
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t s0 = (hi + 0x2904u) ^ lo;
    uint32_t s1 = (mid ^ 0x701u) + 2u * hi;
    uint32_t s2 = 5u * s1 ^ (lo + 0xd94cu);
    values[0] = s2 ^ s0;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    uint32_t cur = ((uint32_t)(challenge >> 18) & 0xfu) ^ 0x1du;
    values[0] = cur;
    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t mask = (uint32_t)(challenge >> (26u + 4u * (i - 1))) & 0x1fu;
        cur = 3u * (cur + i) - mask;
        values[i] = cur;
    }
     return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t idx = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    uint32_t r;
    switch (idx) {
        case 0: r = 5u * x + 0x2b5b6u; break;
        case 1: r = (x ^ 0x7401u) + (x >> 2); break;
        case 2: r = 5u * x + 0x854cu; break;
        case 3: r = (4u * x) ^ 0x6de8u; break;
        case 4: r = 5u * x + 0x4fd03u; break;
        case 5: r = (2u * x) ^ 3u; break;
        case 6: r = 3u * x + 0x3685u; break;
        default: r = (x ^ 0xffa5u) + (x >> 1); break;
    }
    values[0] = idx;
    values[1] = r;
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t target, uint32_t lo, uint32_t hi, uint32_t key) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (mid == target) {
        return (mid + 0x56u) ^ key;
    }
    uint32_t node = mid + key;
    uint32_t sub;
    uint32_t result;
    if (target >= mid) {
        sub = p4_walk(target, mid + 1u, hi, key);
        result = (3u * sub) ^ node;
        result ^= 0x8u;
    } else {
        sub = p4_walk(target, lo, mid - 1u, key);
        result = (3u * sub) ^ node;
        result ^= 0x19u;
    }
    return result;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t key = ((uint32_t)(challenge >> 7)) ^ (uint32_t)(challenge >> 32);
    values[0] = p4_walk(target, 0u, 0x7fu, key);
    return write_answer(out, cap, values, count);
}

static const struct {
    uint32_t f0;
    uint32_t f1;
    int next;
} P5_NODES[] = {
    {0x6a1cu, 0x032cu, 7},
    {0xa6ddu, 0x0075u, 5},
    {0x50c6u, 0x057bu, 6},
    {0xd97au, 0x0b3fu, 8},
    {0x63e7u, 0x010bu, 2},
    {0x687eu, 0x0464u, -1},
    {0xaa99u, 0x070eu, 9},
    {0xb7ceu, 0x0644u, 3},
    {0x78d3u, 0x0a0bu, 1},
    {0x3351u, 0x083au, 10},
    {0xdc8bu, 0x0259u, 0},
    {0x1af8u, 0x090fu, -1},
};

static const uint8_t P5_START[8] = {4, 2, 6, 9, 10, 0, 7, 3};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t hi = (uint32_t)(challenge >> 32);
    uint32_t acc = (((uint32_t)(challenge >> 6)) ^ hi) ^ 0x3a63u;
    uint32_t start = P5_START[(uint32_t)challenge & 7u];
    uint32_t hops = ((uint32_t)(challenge >> 3)) & 3u;
    int node = (int)start;
    acc += P5_NODES[node].f0;
    acc = 5u * acc;
    acc ^= P5_NODES[node].f1;
    uint32_t last = P5_NODES[node].f1;
    for (uint32_t k = 1; k <= hops; ++k) {
        node = P5_NODES[node].next;
        acc += P5_NODES[node].f0;
        acc = 5u * acc;
        acc ^= P5_NODES[node].f1;
        last = P5_NODES[node].f1;
    }
    values[0] = acc;
    values[1] = last;
    return write_answer(out, cap, values, count);
}

static uint32_t p6_op(uint32_t op, uint32_t x) {
    switch (op) {
        case 0: return (x + 0x7fecu) ^ (x >> 2);
        case 1: return ror32(x, 3);
        case 2: return ((x << 5) + 0x6925u) ^ (5u * x);
        case 3: return (x ^ 0xa747u) + (x << 7);
        case 4: return (rol32(x, 3)) ^ 0xb309u;
        default: return ((x >> 7) ^ x) + 0xc4f4u;
    }
}

static const uint8_t P6_TAB[8] = {0, 5, 2, 3, 4, 1, 2, 3};

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t v = ((uint32_t)(challenge >> 6)) ^ (uint32_t)(challenge >> 32);
    uint32_t a = p6_op(P6_TAB[(uint32_t)challenge & 7u], v) ^ 0x9f21u;
    a = p6_op(P6_TAB[((uint32_t)challenge >> 3) & 7u], a) + 0x4810u;
    values[0] = a;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t k = (uint32_t)context & 3u;
    uint32_t s1 = (uint32_t)(challenge >> 32) ^ (uint32_t)challenge;
    uint32_t s2 = (uint32_t)(context >> 32) ^ (uint32_t)context;
    uint32_t v = s1 ^ s2;
    uint32_t m = ror32(v, 5) ^ 0x6837u;
    uint32_t t = m ^ ((k * 0xf329u) ^ ((uint32_t)(context >> 16)));
    uint32_t u = t ^ (s2 + k + 0x9b1eu + (s1 >> 5));
    for (int i = 0; i < 32; ++i) out[i] = ' ';
    int n = snprintf(out + 32, cap - 32, "OPEN %" PRIu32 " %" PRIu32 " %" PRIu32, k, t, u);
    return n < 0 || (size_t)n >= cap - 32;
}
