/* SPDX-License-Identifier: MIT */
/* cm-alarm-sites host checks: the compiled site table is sorted and unique,
 * every genuine site resolves under the FB/SUPERBNK that addresses it, and a
 * fixed caller Q that is no site is rejected before any state changes. */
#include <stdio.h>
#include <string.h>
#include "cm_alarm_sites.h"
static unsigned failures, checks;
static void check(int ok, const char *what, unsigned a, unsigned b) {
    ++checks;
    if (!ok) { if (++failures < 20) printf("FAIL %s %o %o\n", what, a, b); }
}
static void base(apollo_overlay_state *s, cm_native_io *io) {
    memset(s, 0, sizeof *s); memset(io, 0, sizeof *io);
    s->words[5] = 05650; s->words[2] = 02000; s->words[0] = 1;
}
int main(void) {
    uint16_t count, i, bank, offset;
    const agc_site_operand *t = cm_alarm_site_table(&count);
    apollo_overlay_state s, before; cm_native_io io; agc_word code;
    check(count == 62u, "site count", count, 62);
    for (i = 1; i < count; ++i) check(t[i - 1].physical < t[i].physical, "sorted", t[i - 1].physical, t[i].physical);
    for (i = 0; i < count; ++i) check(t[i].word <= AGC_MASK && t[i].physical < 40960u, "range", t[i].physical, t[i].word);
    /* Every fixed logical Q under every FB bank and both superbank settings. */
    for (bank = 0; bank < 040u; ++bank) {
        unsigned super;
        for (super = 0; super < 2u; ++super) {
            unsigned q;
            if (super && bank < 030u) continue;
            for (q = 02000u; q <= 07777u; ++q) {
                uint16_t physical = agc_fixed_physical((agc_word)q, (agc_word)(bank << 10), (agc_word)(super ? 0100u : 0u));
                int site = agc_site_lookup(t, count, physical, &code);
                agc_word got = 0;
                enum exec_status status;
                base(&s, &io);
                s.words[4] = (agc_word)(bank << 10); s.words[6] = s.words[4];
                io.superbank = (agc_word)(super ? 0100u : 0u);
                s.words[2] = (agc_word)q;
                check(cm_alarm_site_code(&s, &io, (agc_word)q, &got) == site, "lookup", q, bank);
                if (site) { check(got == code, "operand", q, got); continue; }
                if (q > 07775u) continue;
                before = s;
                status = cm_alarm_site_call(&s, &io, CM_RECORD_ALARM);
                check(status == EXEC_OUT_OF_SCOPE, "non-site rejected", q, bank);
                check(memcmp(&s, &before, sizeof s) == 0, "non-site untouched", q, bank);
                s.words[5] = 07762u; s.words[01363] = (agc_word)q; before = s;
                check(cm_abort_site_call(&s, &io) == EXEC_OUT_OF_SCOPE, "abort non-site", q, bank);
                check(memcmp(&s, &before, sizeof s) == 0, "abort untouched", q, bank);
                s.words[5] = 05652u; s.words[2] = (agc_word)q; before = s;
                check(cm_alarm2_site_call(&s, &io) == EXEC_OUT_OF_SCOPE, "alarm2 non-site", q, bank);
                check(memcmp(&s, &before, sizeof s) == 0, "alarm2 untouched", q, bank);
            }
        }
    }
    /* Banks 040..043 are reached only through SUPERBNK; physical 0..40959. */
    for (offset = 0; offset < 1024u; ++offset)
        check(agc_fixed_physical((agc_word)(02000u + offset), 030u << 10, 0100u) == 040u * 1024u + offset, "superbank", offset, 0);
    check(agc_fixed_physical(01777u, 0, 0) == AGC_TRANSFER_NONE, "erasable", 01777, 0);
    printf("cm-alarm-sites: %u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
