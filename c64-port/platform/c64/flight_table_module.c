/* SPDX-License-Identifier: MIT */
/* A flight-store module (ABI 2, job or task slot): a group of lanes' routine
 * tables, unchanged, run on the transfer ABI from the point the step resolved
 * to this module until a point the group does not hold
 * (tools/flight_store_build.py, docs/FLIGHT-STORE.md). The build passes
 * -include of a generated header that defines FS_BASIC and FS_INTERP, X-macro
 * lists of the group's basic and interpretive tables, and FS_DANZIG (the
 * mission's DANZIG marker table). Value: the step's rk_module_env. */
#include "resident_flight.h"
#define FS_DECLARE(t) extern const agc_routine_table t;
FS_BASIC(FS_DECLARE)
FS_INTERP(FS_DECLARE)
/* Copied from the tables at entry: C99 cannot initialize an array from
 * other objects. */
#define FS_COUNT(t) +1
static agc_routine_table basic[0 FS_BASIC(FS_COUNT)];
#ifdef FS_HAS_INTERP
static agc_routine_table interpretive[1 FS_INTERP(FS_COUNT)];
#endif
#define FS_STEPS 256u
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 rk_io *io=(rk_io *)e->io;
 agc_native_program p=*e->program;
 uint16_t steps;
 enum exec_status status;
 uint8_t n=0;
 (void)selector;
#define FS_COPY_BASIC(t) basic[n++]=t;
 FS_BASIC(FS_COPY_BASIC)
#ifdef FS_HAS_INTERP
 n=0;interpretive[n++]=FS_DANZIG;
#define FS_COPY_INTERP(t) interpretive[n++]=t;
 FS_INTERP(FS_COPY_INTERP)
#endif
 p.basic=basic;p.basic_count=(uint8_t)(sizeof basic/sizeof basic[0]);
#ifdef FS_HAS_INTERP
 p.interpretive=interpretive;p.interpretive_count=(uint8_t)(sizeof interpretive/sizeof interpretive[0]);
#else
 p.interpretive=0;p.interpretive_count=0;
#endif
 status=agc_transfer_run(&p,f->state,io,&io->superbank,AGC_TRANSFER_NONE,FS_STEPS,&steps);
 /* An unmapped point after at least one routine is where the group hands
  * back; the next step resolves it. */
 if(status==EXEC_UNMAPPED&&steps)status=EXEC_OK;
 if(status==EXEC_OUT_OF_SCOPE&&steps==FS_STEPS)status=EXEC_OK;
 e->status=status;
 return APOLLO_OV2_DONE;
}
