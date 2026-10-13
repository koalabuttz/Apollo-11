/* SPDX-License-Identifier: MIT */
/* A flight-store module (ABI 2, job or task slot): a group of lanes' routine
 * tables, unchanged, run on the transfer ABI from the point the step resolved
 * to this module until a point the group does not hold
 * (tools/flight_store_build.py, docs/FLIGHT-STORE.md). The build passes
 * -include of a generated header that defines FS_BASIC and FS_INTERP, X-macro
 * lists of the group's basic and interpretive tables, and FS_DANZIG (the
 * mission's DANZIG marker table). A chunk of one basic table instead
 * defines FS_CHUNK_TABLE and FS_CHUNK, an X-macro list of the entry indices
 * the chunk holds (consecutive, so still sorted by key), and an empty
 * FS_BASIC: the entries are copied by constant index, so LTO folds the
 * loads and links only the chunk's routines. FS_SITES lists the inline
 * operands the module's sources compile ({physical, word} pairs, sorted):
 * the module's routines read their callers' operands from it, not from the
 * resident's Executive site table. FS_FIXED_DATA names the module's
 * fixed_data hook (the self-check module's rope image). Value: the step's
 * rk_module_env. */
#include "resident_flight.h"
#ifdef FS_FIXED_DATA
#include "self_check.h"
#endif
#ifdef FS_SITES
static const agc_site_operand sites[]={FS_SITES};
#endif
#define FS_DECLARE(t) extern const agc_routine_table t;
FS_BASIC(FS_DECLARE)
FS_INTERP(FS_DECLARE)
/* Copied from the tables at entry: C99 cannot initialize an array from
 * other objects. */
#define FS_COUNT(t) +1
#ifdef FS_CHUNK
extern const agc_routine_table FS_CHUNK_TABLE;
static agc_routine_entry chunk[0 FS_CHUNK(FS_COUNT)];
static agc_routine_table basic[1];
#else
static agc_routine_table basic[0 FS_BASIC(FS_COUNT)];
#endif
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
#ifdef FS_CHUNK
#define FS_COPY_ENTRY(i) chunk[n++]=FS_CHUNK_TABLE.entries[i];
 FS_CHUNK(FS_COPY_ENTRY)
 basic[0].entries=chunk;basic[0].count=n;
#else
#define FS_COPY_BASIC(t) basic[n++]=t;
 FS_BASIC(FS_COPY_BASIC)
#endif
#ifdef FS_HAS_INTERP
 n=0;interpretive[n++]=FS_DANZIG;
#define FS_COPY_INTERP(t) interpretive[n++]=t;
 FS_INTERP(FS_COPY_INTERP)
#endif
#ifdef FS_SITES
 p.operands=sites;p.operand_count=(uint16_t)(sizeof sites/sizeof sites[0]);
#endif
#ifdef FS_FIXED_DATA
 p.fixed_data=FS_FIXED_DATA;
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
