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
 * fixed_data hook (the self-check module's rope image); FS_ROPE_DATA gives
 * it every rope word from the REU image instead. FS_IO_KIND names the io
 * record the group's tables take (include/flight_io.h) where it is not the
 * mission's base record (rk_io), and FS_IO_CM is 1 when that record starts
 * with cm_native_io: the module then runs them on that record, written from
 * the resident's (rk_device, rk_hardware and rk_flight)
 * and written back after, and is never called directly. Value: the step's
 * rk_module_env. */
#include "resident_flight.h"
#ifdef FS_IO_KIND
#include "flight_io.h"
#endif
#ifdef FS_FIXED_DATA
#include "self_check.h"
#endif
#ifdef FS_ROPE_DATA
#include "rope_image.h"
static bool rope_data(const agc_native_program *p,uint16_t physical,agc_word *word){
 (void)p;
#if RK_LM
 return apollo_rope_read(0u,physical,word);
#else
 return apollo_rope_read(1u,physical,word);
#endif
}
#define FS_FIXED_DATA rope_data
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
/* The module's table list (resident_flight.h), named by the header's TABLES
 * word: built on the first run after a load, then read in place by the
 * resident, which calls this module's routines directly while it stays
 * loaded. */
rk_module_tables rk_module_tables_of_module;
/* The header's TABLES word (platform/c64/overlay_v2_payload.ld). */
__attribute__((used,section(".ov2_tables"))) rk_module_tables *const rk_module_tables_word=&rk_module_tables_of_module;
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 rk_io *io=(rk_io *)e->io;
 rk_module_tables *t=&rk_module_tables_of_module;
 agc_native_program p=*e->program;
 uint16_t steps;
 enum exec_status status;
 (void)selector;
 if(!t->ready){
  uint8_t n=0;
#ifdef FS_CHUNK
/* Field by field: a whole-struct copy lowers to a memcpy from the entries
 * array, which keeps every routine linked (Lane F). */
#define FS_COPY_ENTRY(i) chunk[n].physical=FS_CHUNK_TABLE.entries[i].physical;chunk[n++].call=FS_CHUNK_TABLE.entries[i].call;
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
  t->interpretive=interpretive;t->interpretive_count=(uint8_t)(sizeof interpretive/sizeof interpretive[0]);
#endif
#ifdef FS_SITES
  t->operands=sites;t->operand_count=(uint16_t)(sizeof sites/sizeof sites[0]);
#else
  t->operands=p.operands;t->operand_count=p.operand_count;
#endif
#ifdef FS_FIXED_DATA
  t->fixed_data=FS_FIXED_DATA;
#else
  t->fixed_data=p.fixed_data;
#endif
  t->basic=basic;t->basic_count=(uint8_t)(sizeof basic/sizeof basic[0]);
#ifdef FS_IO_KIND
  t->ready=RK_TABLES_NOT_DIRECT;
#else
  t->ready=RK_TABLES_DIRECT;
#endif
 }
 rk_module_program(&p,t);
#ifdef FS_IO_KIND
 {
  static flight_io_buffer b;   /* static: absolute addressing is smaller than the soft stack */
  agc_hardware *const hw=&rk_hardware;
  void *x;
  /* An LM_* or CM_* record opens with rk_io, which is copied in whole, so
   * its base fields stay rk_device's; CM tables on p66_bias_io take the
   * superbank and interrupt latch. One call each, so LTO folds the kind's
   * case. */
  x=flight_io_enter(FS_IO_KIND,&rk_flight,hw,&b);
#if FS_IO_CM==RK_CM
  memcpy(&b,io,sizeof *io);
#else   /* CM tables on p66_bias_io */
  b.p66.superbank=io->superbank;b.p66.interrupt_enabled=io->interrupt_enabled;
#endif
#if FS_IO_CM
  status=agc_transfer_run(&p,f->state,x,&((cm_native_io *)x)->superbank,AGC_TRANSFER_NONE,RK_MODULE_STEPS,&steps);
#else
  status=agc_transfer_run(&p,f->state,x,&((p66_bias_io *)x)->superbank,AGC_TRANSFER_NONE,RK_MODULE_STEPS,&steps);
#endif
  flight_io_leave(FS_IO_KIND,&rk_flight,hw,&b);
#if FS_IO_CM==RK_CM
  memcpy(io,&b,sizeof *io);
#else
  io->superbank=b.p66.superbank;io->interrupt_enabled=b.p66.interrupt_enabled;
#endif
 }
#else
 status=agc_transfer_run(&p,f->state,io,&io->superbank,AGC_TRANSFER_NONE,RK_MODULE_STEPS,&steps);
#endif
 e->status=rk_module_status(status,steps,RK_MODULE_STEPS);
 return APOLLO_OV2_DONE;
}
