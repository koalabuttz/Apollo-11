/* SPDX-License-Identifier: MIT */
/* MYSUBS.agc of Comanche055 (CM), listing pp. 999-1001: SMALLMP and
 * DPADD, the RCS DAP's double-length arithmetic on KMPAC (cm_dap.h). The
 * faithful C is c64-port/src/cm_mysubs.c. */
#include "cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* KMPAC's two words: the upper, then the lower. */
#define KMPAC_HIGH ADDRESS_OF(KMPAC)
#define KMPAC_LOW  (ADDRESS_OF(KMPAC) + 1u)

/* SMALLMP -- p. 999.
 * A single word times the double word in KMPAC, "A(X+Y)": the product of
 * the lower word, "AY", then that of the upper one, "AX", added in as a
 * double word, "AX+AY". (TS KMPTEMP overflows only when the caller's A
 * holds an overflow: it skips the EXTEND, and MP KMPAC +1 runs as MASK.) */
void smallmp(void)
{
    if (ts(ADDRESS_OF(KMPTEMP)))
        mask_at(KMPAC_LOW);
    else
        mp_at(KMPAC_LOW);
    ts(KMPAC_LOW); /* "AY" */
    set_A(ZERO);   /* CAF */
    xch(KMPAC_HIGH);
    mp_at(ADDRESS_OF(KMPTEMP)); /* "AX" */
    das(KMPAC_HIGH);            /* "AX+AY" */
    tc_through(Q_ADDRESS);      /* TC Q */
}

/* DPADD -- pp. 999-1000.
 * A,L added into KMPAC. When the sum overflows, it is taken round: the
 * upper word gets NEGMAX or POSMAX added the other way (DPADD+ and DPADD-),
 * or, with the upper word zero, the lower word does and the upper word
 * becomes NEGMAX or POSMAX (DPADD2+, and the steps after CCS KMPAC +1). */
void dpadd(void)
{
    enum sign lower;

    das(KMPAC_HIGH);
    if (A_is_zero()) {         /* EXTEND; BZF TSK +1: "NO OVERFLOW" */
        tc_through(Q_ADDRESS); /* TC Q */
        return;
    }
    switch (ccs_at(KMPAC_HIGH)) {
    case POSITIVE:  /* "+ OVERFLOW" */
        ad(NEGMAX); /* DPADD+: "KMPAC GREATER THAN 0" */
        break;
    case NEGATIVE:  /* "- OVERFLOW" */
        com();      /* DPADD- */
        ad(POSMAX); /* "KMPAC LESS THAN 0" */
        break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO:
        lower = ccs_at(KMPAC_LOW);
        if (lower == POSITIVE) { /* "UPPER = 0, LOWER +" */
            ad(NEGMAX);          /* DPADD2+ */
            ts(KMPAC_LOW);       /* "CAN NOT OVERFLOW" */
            set_A(NEGMAX);       /* CA: "UPPER WAS = 0" */
        } else {
            if (lower == NEGATIVE) com(); /* "UPPER = 0, LOWER -" */
            ad(POSMAX);                   /* "LOWER = 0, A=0" */
            ts(KMPAC_LOW);                /* "CAN NOT OVERFLOW" */
            set_A(POSMAX);                /* CA: "UPPER WAS = 0" */
        }
        break;
    }
    ts(KMPAC_HIGH);        /* TSK */
    tc_through(Q_ADDRESS); /* TC Q */
}

#endif
