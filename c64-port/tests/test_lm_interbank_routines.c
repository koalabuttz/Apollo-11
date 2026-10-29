/* SPDX-License-Identifier: MIT */
/* lm-interbank-transfer host checks.
 * 1. Every genuine BANKCALL/IBNKCALL/POSTJUMP/USPRCADR site in Luminary099,
 *    in each context: the registry routine at the callee, with the compiled
 *    operand, gives exactly the image of lm_interbank fed the original rope
 *    word (lm_interbank is certified against the assembly per entry).
 * 2. Round trip through the trampoline: caller site -> BANKCALL/IBNKCALL ->
 *    native callee routine -> SWRETURN/ISWRETRN -> caller continuation.
 * 3. A fixed Q that is no site, and an unmapped destination, are refused. */
#include <stdio.h>
#include <string.h>
#include "lm_interbank_routines_cases.h"
#include "lm_fixed_compat.h"
static unsigned failures, checks, entered;
static void check(int ok, const char *what, unsigned long n) { ++checks; if (!ok && ++failures < 20) printf("FAIL %s case %lu\n", what, n); }
static enum exec_status callee(const agc_native_program *p, apollo_overlay_state *s, void *io) {
    (void)p; (void)io; ++entered;
    s->words[0] = 07070; s->words[5] = s->words[2]; /* body, then TC Q (RETURN) */
    return EXEC_OK;
}
int main(void) {
    uint32_t n, cases = lm_ibr_cases();
    for (n = 0; n < cases; ++n) {
        apollo_overlay_state s, ref; p66_bias_io io, rio;
        agc_word callee_z = lm_ibr_input(&s, &io, n), literal = 0, cadr;
        enum lm_interbank_entry entry = lm_ibr_entry(callee_z);
        agc_routine r = agc_transfer_resolve(&lm_ibr_program, &s, io.superbank);
        int ok;
        enum exec_status status;
        ref = s; rio = io;
        check(lm_fixed_compat_read(lm_ibr_site(n), &literal), "rope", n);
        ok = lm_interbank(&ref, &rio, entry, literal, 0);
        check(r != 0, "resolve", n);
        if (!r) continue;
        status = r(&lm_ibr_program, &s, &io);
        check((status == EXEC_OK) == ok, "status", n);
        check(memcmp(&s, &ref, sizeof s) == 0 && memcmp(&io, &rio, sizeof io) == 0, "image", n);
        if (!ok) continue;
        cadr = literal;
        check(s.words[5] == (agc_word)(02000u + (cadr & 01777u)) && s.words[4] == (agc_word)(cadr & 076000u), "destination", n);
        /* Round trip with a native callee registered at the destination. */
        if (entry == LM_IB_BANKCALL || entry == LM_IB_IBNKCALL) {
            agc_routine_entry callee_entry[1];
            agc_routine_table tables[2];
            agc_native_program program = lm_ibr_program;
            uint16_t steps = 0, stop = (uint16_t)(lm_ibr_site(n) + 1u);
            apollo_overlay_state t, expect; p66_bias_io tio, eio;
            lm_ibr_input(&t, &tio, n);
            expect = t; eio = tio;
            callee_entry[0].physical = agc_fixed_physical((agc_word)(02000u + (cadr & 01777u)), cadr, tio.superbank);
            callee_entry[0].call = callee;
            tables[0] = lm_interbank_routines; tables[1].entries = callee_entry; tables[1].count = 1;
            program.basic = tables; program.basic_count = 2;
            entered = 0;
            status = agc_transfer_run(&program, &t, &tio, &tio.superbank, stop, 16, &steps);
            check(status == EXEC_OK && steps == 3 && entered == 1, "round trip", n);
            /* Same composition by hand from the certified entries. */
            lm_interbank(&expect, &eio, entry, literal, 0);
            expect.words[0] = 07070; expect.words[5] = expect.words[2];
            check(lm_interbank(&expect, &eio, entry == LM_IB_BANKCALL ? LM_IB_SWRETURN : LM_IB_ISWRETRN, 0, 0), "return entry", n);
            check(memcmp(&t, &expect, sizeof t) == 0 && memcmp(&tio, &eio, sizeof tio) == 0, "round trip image", n);
        } else {
            uint16_t steps = 0;
            apollo_overlay_state t; p66_bias_io tio;
            lm_ibr_input(&t, &tio, n);
            status = agc_transfer_run(&lm_ibr_program, &t, &tio, &tio.superbank, AGC_TRANSFER_NONE - 1u, 16, &steps);
            check(status == EXEC_UNMAPPED && steps == 1 && t.words[5] == s.words[5], "unmapped destination", n);
        }
    }
    /* A fixed Q that is no compiled site is refused before any change. */
    {
        apollo_overlay_state s, before; p66_bias_io io;
        uint16_t count, q, refused = 0;
        const agc_site_operand *t = lm_interbank_site_table(&count);
        agc_word w;
        lm_ibr_input(&s, &io, 0);
        for (q = 04000; q < 07776; ++q) {
            if (agc_site_lookup(t, count, agc_fixed_physical(q, s.words[4], io.superbank), &w)) continue;
            s.words[5] = 04616; s.words[2] = q; before = s;
            check(agc_transfer_resolve(&lm_ibr_program, &s, io.superbank)(&lm_ibr_program, &s, &io) == EXEC_OUT_OF_SCOPE
                  && memcmp(&s, &before, sizeof s) == 0, "non-site", q);
            ++refused;
        }
        check(refused > 2000u, "non-site sweep", refused);
    }
    printf("lm-interbank-transfer: %lu cases, %u checks, %u failures\n", (unsigned long)cases, checks, failures);
    return failures ? 1 : 0;
}
