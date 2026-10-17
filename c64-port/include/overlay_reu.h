/* SPDX-License-Identifier: MIT */
#ifndef OVERLAY_REU_H
#define OVERLAY_REU_H
/* 17xx RAM Expansion Unit module store for overlay ABI 2. The REU holds a
 * packed store image (tools/overlay_v2_pack.py); the C64 keeps only the
 * resident runtime and fetches module images by DMA. OVERLAY_ABI_V2.md
 * defines the image format. */
#include <stdint.h>
#include "overlay_v2.h"

/* RAM Expansion Controller registers, offsets from $DF00. */
enum {
    APOLLO_REU_STATUS=0, APOLLO_REU_COMMAND=1, APOLLO_REU_C64_LO=2, APOLLO_REU_C64_HI=3,
    APOLLO_REU_REU_LO=4, APOLLO_REU_REU_HI=5, APOLLO_REU_REU_BANK=6,
    APOLLO_REU_LEN_LO=7, APOLLO_REU_LEN_HI=8, APOLLO_REU_IRQ_MASK=9, APOLLO_REU_ADDR_CONTROL=10
};
/* Execute now (no $FF00 trigger); transfer type in bits 1..0. */
#define APOLLO_REU_STASH 0x90u
#define APOLLO_REU_FETCH 0x91u
#define APOLLO_REU_VERIFY 0x93u
#define APOLLO_REU_STATUS_END 0x40u
#define APOLLO_REU_STATUS_FAULT 0x20u

/* Platform hooks: register access and the C64 bus address of a buffer.
 * platform/c64/overlay_reu_platform.c maps them to $DF00 and the pointer
 * value; host tests map them to a simulated controller and C64 RAM. */
void apollo_reu_poke(uint8_t reg,uint8_t value);
uint8_t apollo_reu_peek(uint8_t reg);
uint16_t apollo_reu_c64_address(const void *);

/* One DMA. length 0 means 65,536 bytes, as on the controller. Returns the
 * status register read after the transfer (reading clears it). Fetch writes
 * C64 RAM even under ROM; never target $D000..$DFFF with I/O mapped. */
uint8_t apollo_reu_transfer(uint8_t command,void *c64,uint32_t reu,uint16_t length);

#define APOLLO_REU_MAGIC_BYTES 4u
#define APOLLO_REU_FORMAT 1u
#define APOLLO_REU_HEADER_BYTES 32u
#define APOLLO_REU_INDEX_RECORD 16u
#define APOLLO_REU_INDEX_ENTRIES 4096u
#define APOLLO_REU_RESOLVER_RECORD 8u

typedef struct {
    uint32_t base;
    uint16_t services,index_page,resolver_page,resolver_count,module_count,pages;
    uint8_t index_blocks;   /* index length in blocks of 256 records (16 pages) */
    uint8_t mission;
    uint8_t *record;   /* 16-byte DMA buffer in C64 RAM */
    uint16_t (*invoke)(void *,uint8_t *,apollo_ov2_frame *,uint16_t);
    void *invoke_user;
} apollo_reu_store;

/* Read and check the store header at REU offset base. record is a resident
 * 16-byte DMA buffer (C64 RAM, not I/O). Returns 0 unless the magic, format,
 * mission and service-table version all match and every region fits. */
uint8_t apollo_reu_store_open(apollo_reu_store *,uint32_t base,uint8_t mission,
    uint16_t services,uint8_t *record);
/* Boot-time integrity pass: every present module's header and CRC, and the
 * resolver's ordering. buffer is 256 bytes of C64 RAM (scratch). */
enum apollo_ov2_status apollo_reu_store_verify(apollo_reu_store *,uint8_t *buffer);
/* Boot-time: load(user, REU address, entry) for every slot-class-2 module
 * (fixed extension, APOLLO_OV2_SLOT_FIXED), slot-class-3 module (pinned,
 * APOLLO_OV2_SLOT_PINNED) and slot-class-4 module (pinned task region,
 * APOLLO_OV2_SLOT_PINNED_TASK; d->slot says which), in id order; 0 if a load
 * fails.
 * The store is verified first, so each entry and image is already checked. */
uint8_t apollo_reu_store_fixed(apollo_reu_store *,
    uint8_t (*load)(void *,uint32_t,const apollo_ov2_dirent *),void *user);
apollo_ov2_store apollo_reu_store_backend(apollo_reu_store *,
    uint16_t (*invoke)(void *,uint8_t *,apollo_ov2_frame *,uint16_t),void *invoke_user);
#endif
