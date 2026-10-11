/* SPDX-License-Identifier: MIT
 * 17xx REU register access and ABI 2 module invocation. The ABI 2 runtime
 * memory configuration is $01=$35: BASIC and KERNAL banked out (RAM under
 * both slots), I/O visible for $DF00. The resident owns the RAM IRQ/NMI
 * vectors at $FFFA..$FFFF; slot T ends at $FFF9 so a load never touches them. */
#include <stdint.h>
#include "overlay_reu.h"
void apollo_reu_poke(uint8_t reg,uint8_t value) { ((volatile uint8_t *)0xdf00u)[reg]=value; }
uint8_t apollo_reu_peek(uint8_t reg) { return ((volatile uint8_t *)0xdf00u)[reg]; }
uint16_t apollo_reu_c64_address(const void *p) { return (uint16_t)(uintptr_t)p; }
/* Both slots use the same normal C ABI entry at slot base + 24. */
uint16_t c64_ov2_invoke(void *user,uint8_t *slot,apollo_ov2_frame *frame,uint16_t selector) {
    apollo_ov2_entry entry=(apollo_ov2_entry)(uintptr_t)(slot+APOLLO_OV2_HEADER_BYTES);
    (void)user;
    return entry(frame,selector);
}
