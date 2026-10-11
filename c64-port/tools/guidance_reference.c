/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only adapter: actual MP/DAS instructions and original RODCOMP bytes.
 * Only input constructors are linked; no native arithmetic or guidance code. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "guidance_cases.h"
static agc_t baseline;
static unsigned raw(unsigned address) { return (unsigned)*memory(address) & 0177777u; }
int main(int argc, char **argv) {
    uint16_t n;
    if (argc != 2) return 2;
    memset(&machine,0,sizeof machine);
    if (agc_load_binfile(&machine,argv[1]) != 0) return 2;
    baseline = machine; InhibitAlarms = 1;
    for (n = 0; n < GN_CASE_COUNT; ++n) {
        guidance_case c; uint8_t step;
        gn_prepare(&c,n); machine = baseline;
        for (step = 0; step < gn_steps(n); ++step) {
            unsigned cycles, pc = 0, stop;
            unsigned high = c.operation == GN_ROD ? 03644 : 01020;
            unsigned count = c.operation == GN_ROD ? 03746 : 01023;
            unsigned scale = c.operation == GN_ROD ? 03756 : 01024;
            unsigned i;
            uint16_t words[6];
            gn_next(&c,n,step);
            set(high,c.state.desired.high); set(high+1,c.state.desired.low);
            set(count,c.state.count); set(scale,c.state.scale);
            set(high-1,012345); set(high+2,065432); /* destination sentinels */
            *memory(RegA) = (int16_t)c.state.registers.a;
            *memory(RegL) = (int16_t)c.state.registers.l;
            set(RegFB,064000); set(RegEB,03400); set(RegBB,064007);
            set(RegQ,01030); machine.AllowInterrupt = 1;
            if (c.operation == GN_ROD) { set(RegZ,03275); stop = 03303; }
            else if (c.operation == GN_MP) {
                set(01010,000006); set(01011,071024); set(RegZ,01010); stop = 01012;
            } else { set(01010,021021); set(RegZ,01010); stop = 01011; }
            for (cycles = 0; cycles < 100; ++cycles) {
                pc = get(RegZ);
                if (!machine.PendFlag && !machine.ExtraDelay && pc == stop) break;
                machine.ScalerCounter = 0; /* no asynchronous counters in this fixture */
                if (agc_engine(&machine) != 0) return 2;
            }
            if (cycles == 100 || get(high-1) != 012345 || get(high+2) != 065432 ||
                get(RegBB) != 064007 || (c.operation == GN_ROD && machine.AllowInterrupt)) {
                fprintf(stderr,"case %u/%u failed boundary at %05o\n",n,step,pc); return 2;
            }
            c.state.count = (agc_word)get(count); c.state.scale = (agc_word)get(scale);
            c.state.desired.high = (agc_word)get(high); c.state.desired.low = (agc_word)get(high+1);
            c.state.registers.a = (agc_register)raw(RegA); c.state.registers.l = (agc_register)raw(RegL);
            gn_words(&c,words);
            printf("%u %u %u",n,step,(unsigned)c.operation);
            for (i = 0; i < 6; ++i) printf(" %u",words[i]);
            puts("");
            if (n < GN_TARGET_COUNT) fprintf(stderr,"case %u/%u: %u AGC cycles, stop %05o, BB %05o\n",n,step,cycles,pc,get(RegBB));
        }
    }
    return 0;
}
