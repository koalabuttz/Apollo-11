/* SPDX-License-Identifier: MIT */
#include "cm_interbank_postjump.h"
#include "cm_native_image.h"
#include "agc_image_w.h"
/* High byte of word i (both targets are little-endian). */
#define HI(i) ((uint8_t *)&W[i])[1]
/* L through POSTJUMP: overflow-corrected, then sign-extended again (bit 15
 * copied to bit 14). */
#define NORM(i) (HI(i) = (uint8_t)((HI(i) & 0277u) | ((HI(i) >> 1) & 0100u)))
/* POSTJUMP (Z 04701): XCH Q, INDEX A, CA 0, then BANKJUMP's TS FBANK, MASK
 * LOW10, XCH Q, INDEX Q, TCF 10000. The CADR at Q (erasable, EB-relative, or
 * the caller's compiled operand) goes to FB and Z, its low ten bits to Q; A
 * passes through. Same results as cm_interbank_routines' POSTJUMP in the
 * state cm_interbank_complete accepts (EB, FB and BB agree, superbank at most
 * 7, interrupts 0 or 1, Z18 word 7 clear, Q a TC caller's); any other state
 * is out of scope. Reached only through the transfer engine (rk_direct),
 * which passes the program, the image and the io record: no NULL checks. */
static enum exec_status postjump(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    const cm_native_io *io = v;
    const agc_site_operand *o;
    uint16_t n, at;
    agc_word q, t;
    if (W[5] != 04701u || W[7] || (W[3] & ~03400u) || (W[4] & ~076000u) ||
        W[6] != (W[4] | (W[3] >> 8)) || (io->superbank & ~0160u) || io->interrupt_enabled > 1u)
        return EXEC_OUT_OF_SCOPE;
    q = W[2];
    if (q < 0100u || q > 07775u) return EXEC_OUT_OF_SCOPE;
    if (q < 01400u)
        t = W[q];
    else if (q < 02000u) {
        if (W[3] + (q & 0377u) < 0100u) return EXEC_OUT_OF_SCOPE;
        t = W[W[3] + (q & 0377u)];
    } else {
        /* The caller's CADR: a short scan of the program's few sites. */
        at = agc_fixed_physical(q, W[4], io->superbank);
        for (o = p->operands, n = p->operand_count; n && o->physical != at; --n, ++o) {
        }
        if (!n || (t = o->word) > AGC_MASK) return EXEC_OUT_OF_SCOPE;
    }
    NORM(1);
    W[4] = (agc_word)(t & 076000u);
    W[6] = (agc_word)(W[4] | (W[3] >> 8));
    W[2] = (agc_word)(t & 01777u);
    W[5] = (agc_word)(02000u + W[2]);
    return EXEC_OK;
}
#define AT(z) (uint16_t)(2048u + ((z) - 04000u))
static const agc_routine_entry pj[1] = {{AT(04701), postjump}};
const agc_routine_table cm_interbank_postjump_hot_routines = {pj, 1};
