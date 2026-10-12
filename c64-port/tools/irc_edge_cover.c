/* SPDX-License-Identifier: MIT */
/* Edge coverage for tools/interp_routine_native.c's cover mode: the native
 * sources are built with clang -O0 -fsanitize-coverage=trace-pc-guard,
 * trace-cmp, this file without. Features 1..edges are the control-flow
 * edges; after them each comparison site (in the order runs first reach
 * them) has three, its operands equal, less and greater, so a condition
 * whose outcome is only a value (a || b returned) counts too. */
#include <stdint.h>
#include <string.h>
enum { IRC_MAX_EDGES = 1u << 20 };
static uint8_t hit[IRC_MAX_EDGES / 8];
static uint32_t edges;
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start, uint32_t *stop);
void __sanitizer_cov_trace_pc_guard(uint32_t *guard);
void __sanitizer_cov_trace_pc_guard_init(uint32_t *start, uint32_t *stop)
{
    uint32_t *g;
    if (start == stop || *start)
        return;
    for (g = start; g < stop && edges + 1u < IRC_MAX_EDGES / 2u; ++g)
        *g = ++edges;
}
void __sanitizer_cov_trace_pc_guard(uint32_t *guard)
{
    uint32_t k = *guard;
    if (k)
        hit[k >> 3] = (uint8_t)(hit[k >> 3] | 1u << (k & 7u));
}
enum { IRC_SITES = 1u << 14 };
static uintptr_t site_pc[IRC_SITES];
static uint32_t sites;
static void cmp(uintptr_t pc, uint64_t a, uint64_t b)
{
    uint32_t h = (uint32_t)((pc * 0x9e3779b1u) >> 7) & (IRC_SITES - 1u), k;
    while (site_pc[h] && site_pc[h] != pc)
        h = (h + 1u) & (IRC_SITES - 1u);
    if (!site_pc[h]) {
        if (sites >= IRC_SITES / 2u)
            return;
        site_pc[h] = pc;
        ++sites;
    }
    k = IRC_MAX_EDGES / 2u + h * 3u + (a == b ? 0u : (int64_t)a < (int64_t)b ? 1u : 2u);
    hit[k >> 3] = (uint8_t)(hit[k >> 3] | 1u << (k & 7u));
}
#define PC ((uintptr_t)__builtin_return_address(0))
void __sanitizer_cov_trace_cmp1(uint8_t a, uint8_t b);
void __sanitizer_cov_trace_cmp2(uint16_t a, uint16_t b);
void __sanitizer_cov_trace_cmp4(uint32_t a, uint32_t b);
void __sanitizer_cov_trace_cmp8(uint64_t a, uint64_t b);
void __sanitizer_cov_trace_const_cmp1(uint8_t a, uint8_t b);
void __sanitizer_cov_trace_const_cmp2(uint16_t a, uint16_t b);
void __sanitizer_cov_trace_const_cmp4(uint32_t a, uint32_t b);
void __sanitizer_cov_trace_const_cmp8(uint64_t a, uint64_t b);
void __sanitizer_cov_trace_switch(uint64_t v, uint64_t *cases);
void __sanitizer_cov_trace_cmp1(uint8_t a, uint8_t b) { cmp(PC, (int8_t)a, (int8_t)b); }
void __sanitizer_cov_trace_cmp2(uint16_t a, uint16_t b) { cmp(PC, (int16_t)a, (int16_t)b); }
void __sanitizer_cov_trace_cmp4(uint32_t a, uint32_t b) { cmp(PC, (int32_t)a, (int32_t)b); }
void __sanitizer_cov_trace_cmp8(uint64_t a, uint64_t b) { cmp(PC, a, b); }
void __sanitizer_cov_trace_const_cmp1(uint8_t a, uint8_t b) { cmp(PC, (int8_t)a, (int8_t)b); }
void __sanitizer_cov_trace_const_cmp2(uint16_t a, uint16_t b) { cmp(PC, (int16_t)a, (int16_t)b); }
void __sanitizer_cov_trace_const_cmp4(uint32_t a, uint32_t b) { cmp(PC, (int32_t)a, (int32_t)b); }
void __sanitizer_cov_trace_const_cmp8(uint64_t a, uint64_t b) { cmp(PC, a, b); }
void __sanitizer_cov_trace_switch(uint64_t v, uint64_t *cases) { (void)v; (void)cases; }
unsigned irc_edge_count(void);
unsigned irc_edge_byte(unsigned k);
void irc_edges_reset(void);
int irc_edge_hit(unsigned k);
unsigned irc_edge_count(void) { return IRC_MAX_EDGES / 2u + IRC_SITES * 3u; }
unsigned irc_edge_byte(unsigned k) { return k < sizeof hit ? hit[k] : 0u; }
void irc_edges_reset(void) { memset(hit, 0, sizeof hit); }
int irc_edge_hit(unsigned k) { return k < IRC_MAX_EDGES && (hit[k >> 3] >> (k & 7u) & 1u); }
