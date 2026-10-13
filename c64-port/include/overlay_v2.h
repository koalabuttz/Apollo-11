/* SPDX-License-Identifier: MIT */
#ifndef OVERLAY_V2_H
#define OVERLAY_V2_H
/* Native overlay ABI version 2: two fixed slots, a bulk module store (REU or
 * disk), 16-bit lane-allocated module identities and resident continuation
 * trampolining. OVERLAY_ABI_V2.md is the normative contract; ABI 1
 * (overlay_runtime.h) remains for the retained single-slot proofs. */
#include <stdint.h>
#include "overlay_runtime.h"

#define APOLLO_OV2_ABI 2u
#define APOLLO_OV2_HEADER_BYTES 24u
#define APOLLO_OV2_JOB_ADDRESS 0x8000u
#define APOLLO_OV2_JOB_BYTES 0x4000u   /* $8000..$bfff, under BASIC ROM */
#define APOLLO_OV2_TASK_ADDRESS 0xe000u
#define APOLLO_OV2_TASK_BYTES 0x1ffau  /* $e000..$fff9, under KERNAL ROM */
/* Slot class 2: a fixed extension in the RAM under I/O, $d000..$dfff. The
 * resident loads every class-2 module once at boot (apollo_reu_store_fixed)
 * with I/O banked out during the DMA; a run never loads one, and it has no
 * BSS. Its code runs with $01=$34 or $30 (the IRQ entry restores I/O). */
#define APOLLO_OV2_SLOT_FIXED 2u
#define APOLLO_OV2_FIXED_ADDRESS 0xd000u
#define APOLLO_OV2_FIXED_BYTES 0x1000u
/* Native contexts owning a seam stack: eight Executive core-set positions,
 * then one task (interrupt-level) context. */
#define APOLLO_OV2_JOB_CONTEXTS 8u
#define APOLLO_OV2_TASK_CONTEXT 8u
#define APOLLO_OV2_CONTEXTS 9u
#define APOLLO_OV2_SEAM_DEPTH 4u
/* Chained JUMP/CALL/RET transfers allowed in one run before the trampoline
 * reports a runaway chain instead of looping forever. */
#define APOLLO_OV2_CHAIN_LIMIT 64u

enum apollo_ov2_slot { APOLLO_OV2_SLOT_JOB=0, APOLLO_OV2_SLOT_TASK=1, APOLLO_OV2_SLOTS=2 };
enum apollo_ov2_mission { APOLLO_OV2_LM=1, APOLLO_OV2_CM=2 };

/* Module identity: high byte is the owning lane (1..15), low byte is the
 * lane's own module number. Zero is never a module. Lanes allocate inside
 * their own range without coordination. */
#define APOLLO_OV2_ID(lane,number) ((uint16_t)(((uint16_t)(lane)<<8)|(uint8_t)(number)))
#define APOLLO_OV2_LANE(id) ((uint8_t)((id)>>8))
enum apollo_ov2_lane {
    APOLLO_LANE_A=0x1, APOLLO_LANE_B=0x2, APOLLO_LANE_C=0x3, APOLLO_LANE_D1=0x4,
    APOLLO_LANE_D2=0x5, APOLLO_LANE_E=0x6, APOLLO_LANE_F=0x7, APOLLO_LANE_G=0x8,
    APOLLO_LANE_H=0x9, APOLLO_LANE_PROOF=0xf
};
#define APOLLO_OV2_MAX_LANE 0xfu

/* A native continuation: module identity plus an explicit selector into the
 * module's own entry table. Never an AGC address or instruction. */
typedef struct { uint16_t module,selector; } apollo_ov2_cont;

/* Module entry result. Values >= APOLLO_OV2_KERNEL_ACTION are opaque to the
 * overlay layer and handed back to the resident caller (Executive/WAITLIST)
 * with frame->next as the resume continuation. */
enum apollo_ov2_action {
    APOLLO_OV2_DONE=0,      /* body finished; frame->value is the result */
    APOLLO_OV2_JUMP=1,      /* tail transfer to frame->next */
    APOLLO_OV2_CALL=2,      /* push frame->ret on this context's seam stack, go to frame->next */
    APOLLO_OV2_RET=3,       /* pop this context's seam stack and continue there */
    APOLLO_OV2_JUMP_HISTORICAL=4, /* resolve frame->hist_bank/hist_loc to a continuation */
    APOLLO_OV2_KERNEL_ACTION=0x10
};

/* The one argument a module receives. It lives in resident storage; a module
 * must not keep a pointer to it, or to anything in a slot, after returning. */
typedef struct {
    apollo_overlay_state *state;   /* canonical erasable image at $7000 */
    apollo_ov2_cont next,ret;      /* outputs for JUMP/CALL/kernel actions */
    uint16_t hist_bank,hist_loc;   /* output for JUMP_HISTORICAL: BBCON/FBANK word and 12-bit address */
    uint16_t value;                /* input argument and DONE result */
    uint8_t context;               /* 0..7 core-set position, 8 task level */
    uint8_t reserved;
} apollo_ov2_frame;

typedef uint16_t (*apollo_ov2_entry)(apollo_ov2_frame *,uint16_t selector);

/* Store directory entry, as returned by a backend lookup. */
typedef struct {
    uint16_t id,page,bytes,crc16,selectors;
    uint8_t slot,flags;
} apollo_ov2_dirent;
#define APOLLO_OV2_DIRENT_PRESENT 0x01u

enum apollo_ov2_status {
    APOLLO_OV2_OK=0, APOLLO_OV2_BUSY=1, APOLLO_OV2_UNMAPPED=2,
    APOLLO_OV2_BAD_DIRECTORY=3, APOLLO_OV2_IO_ERROR=4, APOLLO_OV2_BAD_HEADER=5,
    APOLLO_OV2_BAD_CHECKSUM=6, APOLLO_OV2_BAD_SELECTOR=7,
    APOLLO_OV2_SEAM_OVERFLOW=8, APOLLO_OV2_SEAM_UNDERFLOW=9,
    APOLLO_OV2_UNRESOLVED=10, APOLLO_OV2_BAD_ACTION=11, APOLLO_OV2_RUNAWAY=12,
    APOLLO_OV2_BAD_CONTEXT=13, APOLLO_OV2_FAULTED=14
};

/* Module store backend. lookup and resolve read only directory/resolver
 * records; fetch copies count bytes of a module image starting at offset
 * into dest. invoke transfers to the entry at slot base+24 with the normal
 * C ABI on the target. Failures return 0 and must not write a slot outside
 * [dest,dest+count). */
typedef struct {
    void *user;
    uint8_t (*lookup)(void *,uint16_t id,apollo_ov2_dirent *);
    uint8_t (*fetch)(void *,const apollo_ov2_dirent *,uint16_t offset,uint8_t *dest,uint16_t count);
    uint8_t (*resolve)(void *,uint16_t bank,uint16_t loc,apollo_ov2_cont *);
    uint16_t (*invoke)(void *,uint8_t *slot,apollo_ov2_frame *,uint16_t selector);
} apollo_ov2_store;

typedef struct {
    apollo_ov2_cont stack[APOLLO_OV2_SEAM_DEPTH];
    uint8_t depth;
} apollo_ov2_seams;

typedef struct {
    apollo_ov2_store store;
    uint8_t *slot[APOLLO_OV2_SLOTS];
    uint16_t loaded[APOLLO_OV2_SLOTS];
    uint16_t selectors[APOLLO_OV2_SLOTS];
    apollo_overlay_state *state;
    apollo_ov2_seams seams[APOLLO_OV2_CONTEXTS];
    apollo_ov2_frame frame;
    uint8_t *header;            /* 24-byte resident buffer, C64 RAM (DMA target) */
    uint16_t services;          /* resident service-table version */
    uint8_t mission,busy,verify,fault;
} apollo_ov2_runtime;

/* No loading occurs. header is a resident 24-byte buffer: a module's header
 * is fetched and checked there first, so a rejected module never disturbs
 * the cached slot. verify!=0 checks each payload CRC on every load; the REU
 * build verifies the whole store at boot instead and passes 0. */
void apollo_ov2_init(apollo_ov2_runtime *,apollo_ov2_store,uint8_t *job_slot,
    uint8_t *task_slot,uint8_t *header,apollo_overlay_state *,uint8_t mission,
    uint16_t services,uint8_t verify);

/* Run a continuation chain on behalf of one context until a body returns DONE
 * or a kernel action. *action receives DONE or the kernel action; on a kernel
 * action, *resume is the continuation the caller re-enters later. Loads
 * happen only here, only while no overlay frame is live. Failures leave the
 * canonical image as the last body committed it, invalidate a partially
 * loaded slot and latch fault so no further run starts until restart. */
enum apollo_ov2_status apollo_ov2_run(apollo_ov2_runtime *,uint8_t context,
    apollo_ov2_cont start,uint16_t *value,uint8_t *action,apollo_ov2_cont *resume);

/* CHANJOB exchanges core-set contents; the Executive calls this with the
 * same two positions so native seams follow their job. */
void apollo_ov2_seam_exchange(apollo_ov2_runtime *,uint8_t a,uint8_t b);
/* ENDOFJOB releases a position: its native seams are discarded. */
void apollo_ov2_seam_release(apollo_ov2_runtime *,uint8_t context);
/* GOJAM/fresh start: drop every native seam, both slot caches and the fault
 * latch. The canonical image is untouched; restart logic rebuilds from it. */
void apollo_ov2_restart(apollo_ov2_runtime *);

/* Header field offsets (little-endian two-byte fields). TABLES is 0, or
 * the address in the loaded image of the module's routine-table list (a
 * flight module's rk_module_tables, platform/c64/resident_flight.h), which
 * the resident reads in place to call a loaded module's routines directly. */
enum {
    APOLLO_OV2_H_MAGIC=0, APOLLO_OV2_H_ABI=4, APOLLO_OV2_H_SIZE=5, APOLLO_OV2_H_SLOT=6,
    APOLLO_OV2_H_MISSION=7, APOLLO_OV2_H_ID=8, APOLLO_OV2_H_ENTRY=10, APOLLO_OV2_H_BSS=12,
    APOLLO_OV2_H_BSS_BYTES=14, APOLLO_OV2_H_LOAD_BYTES=16, APOLLO_OV2_H_SERVICES=18,
    APOLLO_OV2_H_SELECTORS=20, APOLLO_OV2_H_TABLES=22
};
uint16_t apollo_ov2_slot_address(uint8_t slot);
/* Service table versions are major<<8|minor. The table is append-only within
 * a major version, so a module built against an older minor still links to
 * the same addresses. */
uint8_t apollo_ov2_services_compatible(uint16_t module,uint16_t resident);
uint16_t apollo_ov2_slot_bytes(uint8_t slot);
#endif
