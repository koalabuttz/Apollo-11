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
 * loads and links only the chunk's routines. FS_PICK, X(table, index) in key
 * order, does the same across several tables (the entries whose keys no
 * other table resolves first). FS_SITES lists the inline
 * operands the module's sources compile ({physical, word} pairs, sorted):
 * the module's routines read their callers' operands from it, not from the
 * resident's Executive site table. FS_FIXED_DATA names the module's
 * fixed_data hook (the self-check module's rope image); FS_ROPE_DATA gives
 * it every rope word from the REU image instead. FS_IO_KIND names the io
 * record the group's tables take (include/flight_io.h) where it is not the
 * mission's base record (rk_io), and FS_IO_CM is 1 when that record starts
 * with cm_native_io: the module then runs them on that record, written from
 * the resident's (rk_device, rk_hardware, and rk_flight)
 * and written back after, and is never called directly. FS_DIRECT_ONLY
 * (pinned modules, base record only) links no transfer engine: the main
 * only builds the table list and returns EXEC_UNMAPPED (nothing ran), and
 * the resident runs the point through rk_direct in the same step. Value: the step's
 * rk_module_env. */
#include "resident_flight.h"
#ifdef FS_IO_KIND
#include "flight_io.h"
#ifdef FS_DIRECT_ONLY
#error "a direct-only module takes the base record (rk_direct passes rk_device)"
#endif
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
#ifdef FS_PICK
/* Picked entries of several basic tables, in key order (FS_PICK, X(table,
 * index)): the build leaves out the entries whose keys another table
 * resolves first, so their routines are not linked. */
#define FS_PICK_DECLARE(t,i) extern const agc_routine_table t;
FS_PICK(FS_PICK_DECLARE)
#define FS_PICK_COUNT(t,i) +1
static agc_routine_entry chunk[0 FS_PICK(FS_PICK_COUNT)];
static agc_routine_table basic[1];
#elif defined FS_CHUNK
extern const agc_routine_table FS_CHUNK_TABLE;
static agc_routine_entry chunk[0 FS_CHUNK(FS_COUNT)];
static agc_routine_table basic[1];
#else
static agc_routine_table basic[0 FS_BASIC(FS_COUNT)];
#endif
#ifdef FS_HAS_INTERP
static agc_routine_table interpretive[1 FS_INTERP(FS_COUNT)];
#endif
/* The work clock's weight (resident_flight.h): the MCT one of this module's
 * routines takes on average, from the calibration (tools/flight_store_build.py
 * FS_MCT); a run chains no further than the step's budget reaches. */
#ifndef FS_MCT
#define FS_MCT RK_RESIDENT_MCT
#endif
/* The high bytes of the keys the tables hold (tools/flight_store_build.py):
 * rk_direct skips the module for a point outside them. */
#ifndef FS_KEY_LO
#define FS_KEY_LO   0u
#define FS_KEY_SPAN 0xffu
#endif
#ifdef RK_WORK_CLOCK
#ifdef FS_IDLE
/* A module carrying the self-check idle loop beside interrupt code: its
 * run outside an interrupt is the work clock's idle (resident_flight.h). */
#define FS_IDLE_NOW (!rk_hardware.in_interrupt)
#define FS_LIMIT    rk_module_limit(e->budget, FS_IDLE_NOW ? RK_MCT_IDLE : FS_MCT)
#else
#define FS_LIMIT rk_module_limit(e->budget,FS_MCT)
#endif
#else
#define FS_LIMIT RK_MODULE_STEPS
#endif
/* FS_KEYS: the chunk's or pick's physical keys in entry order, which the
 * build reads from the same tables (tools/flight_store_build.py
 * table_keys). Copied as data in one loop, so the per-entry code at entry
 * stores only the call (a store each for the key and call was most of a
 * chunk module's main). */
#ifdef FS_KEYS
static const uint16_t chunk_keys[]={FS_KEYS};
_Static_assert(sizeof chunk_keys/sizeof chunk_keys[0]==sizeof chunk/sizeof chunk[0],"FS_KEYS names every entry");
#define FS_COPY_KEYS {uint8_t k;for(k=0;k<n;++k)chunk[k].physical=chunk_keys[k];}
#else
#define FS_COPY_KEYS
#endif
/* The module's table list (resident_flight.h), named by the header's TABLES
 * word: built on the first run after a load, then read in place by the
 * resident, which calls this module's routines directly while it stays
 * loaded. */
rk_module_tables rk_module_tables_of_module;
/* The header's TABLES word (platform/c64/overlay_v2_payload.ld). */
__attribute__((used,section(".ov2_tables"))) rk_module_tables *const rk_module_tables_word=&rk_module_tables_of_module;
#ifdef FS_PIN_HOME
/* The class-4 module (FS_PIN_HOME): the resident never runs its main, as the
 * class-3 module's table list names its tables (rk_pin_main), so it builds
 * nothing and only keeps its tables linked. */
#define FS_HOME_TABLE(t) &t,
__attribute__((used)) static const agc_routine_table *const home_tables[]={FS_BASIC(FS_HOME_TABLE)};
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 (void)f;(void)selector;(void)rk_module_tables_word;
 return APOLLO_OV2_DONE;
}
#else
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){
 rk_module_env *e=(rk_module_env *)(uintptr_t)f->value;
 rk_io *io=(rk_io *)e->io;
 rk_module_tables *t=&rk_module_tables_of_module;
#ifndef FS_DIRECT_ONLY
 agc_native_program p=*e->program;
#endif
 uint16_t steps;
 enum exec_status status;
 (void)selector;
 if(!t->ready){
  uint8_t n=0;
#ifdef FS_PICK
#ifdef FS_KEYS
#define FS_COPY_PICK(t,i) chunk[n++].call=t.entries[i].call;
#else
#define FS_COPY_PICK(t,i) chunk[n].physical=t.entries[i].physical;chunk[n++].call=t.entries[i].call;
#endif
  FS_PICK(FS_COPY_PICK)
  FS_COPY_KEYS
  basic[0].entries=chunk;basic[0].count=n;
#elif defined FS_CHUNK
/* Field by field: a whole-struct copy lowers to a memcpy from the entries
 * array, which keeps every routine linked (Lane F). */
#ifdef FS_KEYS
#define FS_COPY_ENTRY(i) chunk[n++].call=FS_CHUNK_TABLE.entries[i].call;
#else
#define FS_COPY_ENTRY(i) chunk[n].physical=FS_CHUNK_TABLE.entries[i].physical;chunk[n++].call=FS_CHUNK_TABLE.entries[i].call;
#endif
  FS_CHUNK(FS_COPY_ENTRY)
  FS_COPY_KEYS
  basic[0].entries=chunk;basic[0].count=n;
#else
/* Field by field, as above: a struct copy is a memcpy call each. */
#define FS_COPY_BASIC(t) basic[n].entries=t.entries;basic[n++].count=t.count;
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
  t->operands=e->program->operands;t->operand_count=e->program->operand_count;
#endif
#ifdef FS_FIXED_DATA
  t->fixed_data=FS_FIXED_DATA;
#else
  t->fixed_data=e->program->fixed_data;
#endif
  t->basic=basic;t->basic_count=(uint8_t)(sizeof basic/sizeof basic[0]);
  t->mct = FS_MCT;
#ifdef FS_IDLE
  t->idle = 1;
#endif
  t->lo = FS_KEY_LO;
  t->span = FS_KEY_SPAN;
#ifdef FS_IO_KIND
  t->ready=RK_TABLES_NOT_DIRECT;
#else
  t->ready=RK_TABLES_DIRECT;
#endif
 }
 /* A direct-only module runs nothing here (the resident calls its
  * routines through rk_direct): no program to build. */
#ifdef FS_DIRECT_ONLY
 (void)io;(void)steps;(void)status;
 e->status=EXEC_UNMAPPED;
 return APOLLO_OV2_DONE;
#else
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
  status=agc_transfer_run(&p,f->state,x,&((cm_native_io *)x)->superbank,AGC_TRANSFER_NONE,FS_LIMIT,&steps);
#else
  status=agc_transfer_run(&p,f->state,x,&((p66_bias_io *)x)->superbank,AGC_TRANSFER_NONE,FS_LIMIT,&steps);
#endif
  flight_io_leave(FS_IO_KIND,&rk_flight,hw,&b);
#if FS_IO_CM==RK_CM
  memcpy(io,&b,sizeof *io);
#else
  io->superbank=b.p66.superbank;io->interrupt_enabled=b.p66.interrupt_enabled;
#endif
 }
#else
 status=agc_transfer_run(&p,f->state,io,&io->superbank,AGC_TRANSFER_NONE,FS_LIMIT,&steps);
#endif
 e->status=rk_module_status(status,steps,FS_LIMIT);
#ifdef RK_WORK_CLOCK
#ifdef FS_IDLE
 /* The idle loop outside an interrupt, the lead-ins and T4RUPT inside one. */
 e->work = FS_IDLE_NOW ? e->budget : (uint16_t)(steps * FS_MCT);
#else
 e->work=FS_MCT==RK_MCT_IDLE ? e->budget : (uint16_t)(steps*FS_MCT);
#endif
#endif
 return APOLLO_OV2_DONE;
#endif
}
#endif
