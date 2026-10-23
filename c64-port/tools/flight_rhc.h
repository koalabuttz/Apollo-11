/* SPDX-License-Identifier: GPL-2.0-or-later
 * The LM rotational hand controller (RHC, the ACA) for
 * tools/flight_host_run.c (host only, NEVER part of the native port;
 * docs/FLIGHT-STORE.md, Flight scenarios). Included after the yaAGC state m.
 *
 * FLIGHT_RHC=FROM-TO:P,Q,R[;FROM-TO:P,Q,R...] (centiseconds, in time order;
 * signed decimal counts, at most 42 each): the stick, the same on both
 * sides. From the first sample point at or after FROM to the first at or
 * after TO it is out of detent, deflected P, Q and R counts:
 * - channel 31 bit 15 reads 0 (out of detent; active 0), and 1 again after;
 * - at each such sample point where channel 13 bits 8 and 9 are on (the
 *   DAP's ZEROENBL: counters zeroed and enabled, the next read started),
 *   P-RHCCTR, Q-RHCCTR and R-RHCCTR (043, 042, 044) hold the counts, as
 *   yaAGC loads them on that channel 13 write (SocketAPI.c ChannelOutput).
 * The DAP reads them on its next pass (P-AXIS_RCS_AUTOPILOT RATERROR, the
 * Q,R axes alike) in ATTITUDE HOLD (channel 31 bit 13 at 0, set with
 * FLIGHT_CH31 or FLIGHT_CHAN). In detent the counters are left to the DAP.
 * The replay rows carry the counter and channel 31 writes to the C64. */
enum { RHC_SPANS = 16, P_RHCCTR = 043, Q_RHCCTR = 042, R_RHCCTR = 044 };
static struct { unsigned long from, to; int count[3]; } rhc[RHC_SPANS];
static unsigned nrhc;
static int rhc_parse(const char *t)
{
    while (t && *t) {
        unsigned long from, to;
        int p, q, r, n = 0;
        if (sscanf(t, "%lu-%lu:%d,%d,%d%n", &from, &to, &p, &q, &r, &n) < 5 || !n || nrhc >= RHC_SPANS || to <= from ||
            (nrhc && from < rhc[nrhc - 1].to) || p < -42 || p > 42 || q < -42 || q > 42 || r < -42 || r > 42)
            return 0;
        rhc[nrhc].from = from;
        rhc[nrhc].to = to;
        rhc[nrhc].count[0] = p;
        rhc[nrhc].count[1] = q;
        rhc[nrhc++].count[2] = r;
        t += n;
        if (*t == ';') ++t;
    }
    return 1;
}
/* A count as the counter holds it: ones' complement, 15 bits. */
static agc_word rhc_word(int v)
{
    return v < 0 ? (agc_word)(~(unsigned)(-v) & 077777u) : (agc_word)v;
}
/* The stick at sample point t: into yaAGC (hw 0) or the native side. */
static void rhc_due(unsigned long t, agc_word *w, agc_hardware *hw)
{
    unsigned i;
    const int *c = 0;
    agc_word ch13, ch31;
    if (!nrhc) return;
    for (i = 0; i < nrhc; ++i)
        if (t >= rhc[i].from && t < rhc[i].to) c = rhc[i].count;
    ch13 = hw ? hw->channel13 : (agc_word)m.InputChannel[013];
    ch31 = hw ? hw->channel31 : (agc_word)m.InputChannel[031];
    ch31 = c ? (agc_word)(ch31 & ~040000u) : (agc_word)(ch31 | 040000u);
    if (hw)
        hw->channel31 = ch31;
    else
        m.InputChannel[031] = (int16_t)ch31;
    if (!c || (ch13 & 0600u) != 0600u) return;
    eset(w, P_RHCCTR, rhc_word(c[0]));
    eset(w, Q_RHCCTR, rhc_word(c[1]));
    eset(w, R_RHCCTR, rhc_word(c[2]));
}
