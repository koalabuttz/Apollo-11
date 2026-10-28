/* SPDX-License-Identifier: MIT */
#ifndef HAND_CONTROLLER_H
#define HAND_CONTROLLER_H
/* The LM's rotational hand controller (the stick), for the x86 flight
 * program (docs/X86-FLIGHT.md). Same input as the harness's FLIGHT_RHC:
 *
 *   FLIGHT_RHC=FROM-TO:P,Q,R[;FROM-TO:P,Q,R...]
 *
 * centiseconds in time order, P, Q and R signed counts of at most 42. From
 * the first sample point at or after FROM to the first at or after TO the
 * stick is out of detent:
 * - channel 31 bit 15 reads 0 (out of detent), and 1 again after;
 * - at each such sample point where channel 13 bits 8 and 9 are both on
 *   (the DAP has zeroed and enabled the counters), the counters P-RHCCTR,
 *   Q-RHCCTR and R-RHCCTR (043, 042, 044) hold the counts. The DAP reads
 *   them on its next pass in ATTITUDE HOLD.
 * In detent the counters are left to the DAP. With no FLIGHT_RHC the model
 * does nothing. */
#include <stdio.h>
#include "agc_word.h"

enum { HC_SPANS = 16, HC_P = 043, HC_Q = 042, HC_R = 044, HC_MOST = 42 };

typedef struct {
    unsigned long from, to;
    int count[3]; /* P, Q, R */
} hc_span;

typedef struct {
    hc_span span[HC_SPANS];
    unsigned n;
} hc_config;

/* Parse FLIGHT_RHC's value (0 or empty: no stick). 1 on success. */
static int hc_parse(hc_config *c, const char *t)
{
    c->n = 0;
    while (t && *t) {
        hc_span s;
        int used = 0, i;
        if (sscanf(t, "%lu-%lu:%d,%d,%d%n", &s.from, &s.to, &s.count[0], &s.count[1], &s.count[2],
                   &used) < 5 ||
            !used || c->n >= HC_SPANS || s.to <= s.from || (c->n && s.from < c->span[c->n - 1].to))
            return 0;
        for (i = 0; i < 3; ++i)
            if (s.count[i] < -HC_MOST || s.count[i] > HC_MOST) return 0;
        c->span[c->n++] = s;
        t += used;
        if (*t == ';') ++t;
    }
    return 1;
}

/* A signed count as the 15-bit ones' complement counter holds it. */
static agc_word hc_word(int v)
{
    return v < 0 ? (agc_word)(~(unsigned)(-v) & 077777u) : (agc_word)v;
}

/* The stick at sample point t: channel 31's detent bit, and the counters
 * when the DAP has enabled them. */
static void hc_step(const hc_config *c, unsigned long t, agc_word *w, agc_word ch13, agc_word *ch31)
{
    const int *out = 0;
    unsigned i;
    if (!c->n) return;
    for (i = 0; i < c->n; ++i)
        if (t >= c->span[i].from && t < c->span[i].to) out = c->span[i].count;
    *ch31 = out ? (agc_word)(*ch31 & ~040000u) : (agc_word)(*ch31 | 040000u);
    if (!out || (ch13 & 0600u) != 0600u) return;
    w[HC_P] = hc_word(out[0]);
    w[HC_Q] = hc_word(out[1]);
    w[HC_R] = hc_word(out[2]);
}
#endif
