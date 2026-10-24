/* The interrupt hold meter (#838): the longest an interrupt request waits,
 * in machine cycles (MCT), from when it could be taken to the native
 * boundary that takes it. The real AGC takes a request between any two
 * instructions; the port takes it only at a routine boundary (and, since
 * #825, only once a job's part chain has run), so a long routine holds it.
 *
 * The same meter runs on the host (tools/flight_host_run.c, the replay
 * window) and on the C64 (platform/c64/resident_flight.c, -DRK_HOLD_METER),
 * on the work clock, so the two can be compared.
 *
 * A request counts from the later of:
 * - the work clock step that raised it (rupt_hold_raise), or the first
 *   boundary that saw it pending;
 * - the last boundary where the AGC itself could not take it (in an
 *   interrupt, interrupts inhibited, overflow in A). The AGC would take it
 *   somewhere in the next routine, so this is an upper bound.
 * Times are MCT on a 16-bit clock: only differences are used. */
#ifndef RUPT_HOLD_H
#define RUPT_HOLD_H

#include <stdint.h>

typedef struct {
    uint16_t from;   /* when the oldest waiting request could first be taken */
    uint16_t max;    /* the longest hold so far */
    uint16_t max_at; /* TIME1 (low 14 bits) at that take */
    uint8_t on;      /* a request is waiting */
} rupt_hold;

/* A boundary at time t, before the take: pending is the request bits,
 * agc_blocked whether the AGC itself would hold them here. */
static inline void rupt_hold_boundary(rupt_hold *h, uint16_t t, uint16_t pending,
                                      uint8_t agc_blocked)
{
    if (!(pending & 03776u))
        h->on = 0;
    else if (agc_blocked || !h->on) {
        h->from = t;
        h->on = 1;
    }
}

/* The work clock step that ended at time at may have raised a request. */
static inline void rupt_hold_raise(rupt_hold *h, uint16_t at, uint16_t pending)
{
    if ((pending & 03776u) && !h->on) {
        h->from = at;
        h->on = 1;
    }
}

/* A request taken at time t; time1 is TIME1. The next waiting request
 * counts from its next boundary (the interrupt holds it until RESUME). */
static inline void rupt_hold_taken(rupt_hold *h, uint16_t t, uint16_t time1)
{
    uint16_t d = (uint16_t)(t - h->from);
    if (h->on && d > h->max) {
        h->max = d;
        h->max_at = (uint16_t)(time1 & 037777u);
    }
    h->on = 0;
}

#endif
