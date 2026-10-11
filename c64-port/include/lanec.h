/* SPDX-License-Identifier: MIT */
#ifndef LANEC_H
#define LANEC_H
/* Lane C (DSKY, Pinball, extended verbs, display interface) shared types.
 *
 * Native routines run one source path from an entry (a label or a return
 * point) to its first boundary: a transfer to code owned by another routine,
 * file or lane. They work on the canonical erasable image, where the source
 * central registers live at their own addresses, plus the I/O state that
 * Lane C code reads or writes. On return the image and I/O hold exactly what
 * the original program holds when it reaches that boundary, including Z
 * (the boundary address), Q, A/L debris, FB/BB and the superbank. Nothing
 * here fetches, decodes or executes source instructions. */
#include "overlay_runtime.h"
#include "agc_word.h"

#define LANEC_A 0u
#define LANEC_L 1u
#define LANEC_Q 2u
#define LANEC_EB 3u
#define LANEC_FB 4u
#define LANEC_Z 5u
#define LANEC_BB 6u
#define LANEC_CHANNELS 5u /* output channels 010..014 */

typedef struct {
    uint16_t superbank;          /* channel 7 */
    uint16_t interrupt_enabled;  /* 0 after INHINT, 1 after RELINT */
    uint16_t channel[LANEC_CHANNELS];
} lanec_io;

enum lanec_status { LANEC_OK = 0, LANEC_OUT_OF_SCOPE = 1 };
enum lanec_mission { LANEC_LM = 0, LANEC_CM = 1 };

/* Bank-qualified source address: bank is the fixed bank number (0..043) for
 * switchable fixed memory 02000-03777, or LANEC_ANY_BANK for erasable and
 * fixed-fixed addresses. */
#define LANEC_ANY_BANK 0377u
typedef struct { uint16_t bank, lo, hi; } lanec_range;      /* lo..hi inclusive */
typedef struct { uint16_t bank, address; } lanec_point;

/* FB register value for a fixed bank number (banks 040-047 also need the
 * superbank bit 0100 in channel 7). */
uint16_t lanec_fb(uint16_t bank);
/* Fixed bank currently selected by FB and the superbank (0..047). */
uint16_t lanec_current_bank(const apollo_overlay_state *, const lanec_io *);
/* Image registers hold their architectural forms: A/L/Q 16 bits, EB/FB/BB
 * consistent, Z any 12-bit address, ZERO zero. */
uint8_t lanec_registers_valid(const apollo_overlay_state *);
/* Source TC: Q = return address, Z = target. */
void lanec_tc(apollo_overlay_state *, uint16_t target, uint16_t return_address);
/* Source CCS on a 15-bit erasable or fixed word: A <- DABS, returns the
 * skip count 0..3 (positive, +0, negative, -0). */
uint8_t lanec_ccs(apollo_overlay_state *, uint16_t word);
/* Source CCS A on the 16-bit accumulator, overflow included. */
uint8_t lanec_ccs_a(apollo_overlay_state *);
/* Source MASK of A with a 15-bit word. */
void lanec_mask(apollo_overlay_state *, uint16_t word);
/* 15-bit memory word as A's sign-extended 16-bit register form. */
uint16_t lanec_a16(uint16_t word);
/* Source TS of A into erasable word k; returns 1 when A overflowed (A is then
 * +1 or -1 and the source skips). */
uint8_t lanec_ts(apollo_overlay_state *, uint16_t k);

#endif
