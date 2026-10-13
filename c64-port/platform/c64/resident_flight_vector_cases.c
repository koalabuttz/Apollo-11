/* SPDX-License-Identifier: MIT */
/* The flight check's other-vector case module (ABI 2 job slot, part 5 of
 * resident_flight_check.c); frame->value is the address of the check's
 * rk_vector_row. Selector 0 writes other-vector case row->n
 * (tests/resident_vector_cases.c) into the image and row->c and returns
 * value 1. Selector 1, after the check's run: value 1 when
 * rr_vector_digest of the image, row->c and row->kind is the frozen yaAGC
 * digest of that row (tests/resident_rupt_c64_rows.h), else 0. The digest
 * code and the rows live here because the check has no room beside the
 * resident for them. */
#include "resident_flight_check.h"
#include "resident_rupt_c64_rows.h"
#if RK_LM
#define MISSION 0u
#define VECTOR_ROWS rr_lm_c64_vrows
#else
#define MISSION 1u
#define VECTOR_ROWS rr_cm_c64_vrows
#endif
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){rk_vector_row *row=(rk_vector_row *)(uintptr_t)f->value;
 row->c.s=f->state;
 if(selector)f->value=rr_vector_digest(MISSION,&row->c,row->kind)==VECTOR_ROWS[row->n/RR_C64_VSTRIDE];
 else{rr_vector_input(MISSION,row->n,&row->c);f->value=1;}
 return APOLLO_OV2_DONE;
}
