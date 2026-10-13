/* SPDX-License-Identifier: MIT */
/* The flight check's T3RUPT case module (ABI 2 job slot, part 4 of
 * resident_flight_check.c). Selector 0: frame->value is the address of the
 * check's rk_rupt_row; the module writes T3RUPT case row->n
 * (tests/resident_rupt_cases.c) into the image and row->c and returns
 * value 1. */
#include "resident_flight_check.h"
#if RK_LM
#define MISSION 0u
#else
#define MISSION 1u
#endif
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){rk_rupt_row *row=(rk_rupt_row *)(uintptr_t)f->value;
 (void)selector;
 row->c.s=f->state;rr_t3_input(MISSION,row->n,&row->c);
 f->value=1;return APOLLO_OV2_DONE;
}
