/* SPDX-License-Identifier: MIT */
/* The readable interpreter's basic routines (src/interpreter/) as the
 * flight program's: the words of INTERPRETER.agc that basic code in other
 * files enters (DDV/BDDV, SQRTSUB, TPAGREE, POLY, the MAXTEST prologues,
 * INTPRET, NEWMODE and the rest), under the faithful compiled routines'
 * table name and keys (c64-port/src/lm_interpreter.c, cm_interpreter.c):
 * <m>_interpreter_basic. The readable build links it in their place.
 *
 * Plumbing only: point the machine at the program's state, take the
 * states the faithful routines take, and run the readable routine named
 * for the entry. A routine runs until the readable code leaves it (at
 * DANZIG or NEWOPS, a return to its caller, an alarm). Where that is
 * another of these entries, as the faithful routines do, it goes on
 * there. */
#include "binding.h"
#include "rope_image.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099
#define BASIC lm_interpreter_basic
#define ROPE  0u
#else
#define BASIC cm_interpreter_basic
#define ROPE  1u
#endif

/* A routine's key: its physical address. These are all in bank 00 or in
 * fixed-fixed memory. */
#define KEY(z) (uint16_t)((z) >= 04000u ? FF(z) : (z) - 02000u)

/* ---- Fixed memory: the program's rope ---- */

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) return 0u;
    return (word)(value & MINUS_ZERO);
}

/* ---- The routines, by entry ---- */

/* The entries that need a word of their own: MAXTEST -n by n, and the
 * routines whose result the caller reads from the machine. */
static void maxtest_entry(void)
{
    maxtest_minus((unsigned)(MAXTEST - Z));
}
static void sqrtsub_entry(void)
{
    (void)sqrtsub();
}
#if LUMINARY099
static void branch_entry(void)
{
    (void)branch();
}
#endif

/* The entries, in key order: each routine at the word it starts at. */
#define COMMON_HEAD(X)                                                                             \
    X(DVOVF + 6u, ovf_plus_minus1) /* OVF+ -1 */                                                   \
    X(MAXTEST - 21u, maxtest_entry)                                                                \
    X(MAXTEST - 14u, maxtest_entry)                                                                \
    X(MAXTEST - 13u, maxtest_entry)                                                                \
    X(MAXTEST - 12u, maxtest_entry)                                                                \
    X(MAXTEST - 11u, maxtest_entry)                                                                \
    X(MAXTEST - 10u, maxtest_entry)                                                                \
    X(MAXTEST - 9u, maxtest_entry)                                                                 \
    X(MAXTEST - 8u, maxtest_entry)                                                                 \
    X(MAXTEST - 7u, maxtest_entry)                                                                 \
    X(MAXTEST - 6u, maxtest_entry)                                                                 \
    X(MAXTEST - 5u, maxtest_entry)                                                                 \
    X(MAXTEST - 4u, maxtest_entry)                                                                 \
    X(MAXTEST - 3u, maxtest_entry)                                                                 \
    X(MAXTEST - 2u, maxtest_entry)                                                                 \
    X(MAXTEST - 1u, maxtest_entry)                                                                 \
    X(VECAGREE, vecagree)                                                                          \
    X(UNIT, unit)                                                                                  \
    X(UNIT + 1u, unit_plus1)                                                                       \
    X(UNIT + 16u, unit_plus16)                                                                     \
    X(ABS, abs_)                                                                                   \
    X(K_SQRTSUB, sqrtsub_entry)                                                                    \
    X(COSINE, cosine)                                                                              \
    X(K_SINE, sine)                                                                                \
    X(K_SINE + 1u, sine_plus1)                                                                     \
    X(SN1 + 16u, sn1_plus16)                                                                       \
    X(INTPRET, intpret)                                                                            \
    X(INTPRET + 2u, intpret_plus2)                                                                 \
    X(K_INTRSM, intrsm)                                                                            \
    X(K_SLOAD2, sload2)                                                                            \
    X(K_SLOAD2 + 2u, sload2_plus2)                                                                 \
    X(K_NEWMODE, newmode)                                                                          \
    X(K_TLOAD, tload)                                                                              \
    X(K_TLOAD + 6u, tload_plus6)
#if LUMINARY099
#define ROUTINES(X)                                                                                \
    X(K_DDV_BDDV, ddv_bddv)                                                                        \
    COMMON_HEAD(X)                                                                                 \
    X(K_TMODE, tmode)                                                                              \
    X(K_BRANCH, branch_entry)                                                                      \
    X(K_BRANCH + 4u, branch_plus4)                                                                 \
    X(Q_PLUS_1, q_plus_1)                                                                          \
    X(NEG, neg)                                                                                    \
    X(NEG + 1u, neg_plus1)                                                                         \
    X(K_OVERFLOW, overflow)                                                                        \
    X(K_DMP, dmp)                                                                                  \
    X(K_DMPSUB, dmpsub)                                                                            \
    X(K_POWRSERS, powrsers)                                                                        \
    X(K_POLY, poly)                                                                                \
    X(TPAGREE, tpagree)                                                                            \
    X(K_SHORTMP, shortmp)                                                                          \
    X(K_SHORTMP + 1u, shortmp_plus1)                                                               \
    X(K_SHORTMP2, shortmp2)                                                                        \
    X(K_DMPNSUB, dmpnsub)                                                                          \
    X(VXV, vxv)                                                                                    \
    X(K_MPACVBUF, mpacvbuf)                                                                        \
    X(K_ALSIGNAG, alsignag)                                                                        \
    X(K_DCOMP, dcomp)
#else
#define ROUTINES(X)                                                                                \
    COMMON_HEAD(X)                                                                                 \
    X(K_VMODE, vmode)                                                                              \
    X(Q_PLUS_1, q_plus_1)                                                                          \
    X(NEG, neg)                                                                                    \
    X(NEG + 1u, neg_plus1)                                                                         \
    X(K_OVERFLOW, overflow)                                                                        \
    X(K_DMP, dmp)                                                                                  \
    X(K_DMPSUB, dmpsub)                                                                            \
    X(K_POWRSERS, powrsers)                                                                        \
    X(K_POLY, poly)                                                                                \
    X(K_DPAGREE, dpagree)                                                                          \
    X(TPAGREE, tpagree)                                                                            \
    X(K_SHORTMP, shortmp)                                                                          \
    X(K_SHORTMP + 1u, shortmp_plus1)                                                               \
    X(VXV, vxv)                                                                                    \
    X(K_MPACVBUF, mpacvbuf)                                                                        \
    X(K_DCOMP, dcomp)
#endif

#define ROUTINE(z, routine) {(word)(z), routine},
static const struct {
    word z;
    void (*routine)(void);
} entries[] = {ROUTINES(ROUTINE)};
#define ENTRIES (sizeof entries / sizeof entries[0])

/* The entry at the physical address Z names, or ENTRIES. */
static unsigned entry_at(void)
{
    unsigned i, key = Z >= 02000u && Z <= 07777u ? fixed_physical(Z) : 0xFFFFu;

    for (i = 0; i < ENTRIES; ++i)
        if (KEY(entries[i].z) == key) return i;
    return ENTRIES;
}

/* ---- The states the faithful routines take ---- */

/* The bank registers agree (FB is bank 00 for a routine there; fixed-fixed
 * ones take any), ZERO is +0, the latches are in range, and every erasable
 * word is an ordinary one (A, L and Q may hold 16 bits). */
static bool as_expected(word z)
{
    unsigned i;

    if (z < 04000u && FBANK != 0u) return false;
    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    unsigned i;

    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = rope_word;
    i = entry_at();
    if (i == ENTRIES || !as_expected(entries[i].z)) return EXEC_OUT_OF_SCOPE;
    do {
        entries[i].routine();
    } while ((i = entry_at()) != ENTRIES);
    return EXEC_OK;
}

#define BASIC_ENTRY(z, routine) {KEY(z), run},
static const agc_routine_entry basic[] = {ROUTINES(BASIC_ENTRY)};
const agc_routine_table BASIC = {basic, COUNT(basic)};
