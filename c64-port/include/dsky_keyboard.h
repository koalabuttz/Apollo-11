/* SPDX-License-Identifier: MIT */
#ifndef DSKY_KEYBOARD_H
#define DSKY_KEYBOARD_H
#include "p66_basic.h"
#include "dsky_charin_nv.h"
#include "dsky_key_irq.h"
/* Native input transport: C64 PETSCII keys select physical AGC channel15
 * codes. Decimal formatting/key-body behavior belongs to native Pinball.
 * Unknown characters leave the output unchanged. */
uint8_t dsky_keyboard_decode(uint8_t character,uint16_t *channel15);
typedef struct {p66_basic_binding basic;} dsky_keyboard_binding;
#define DSKY_KEYBOARD_MAPPING_COUNT 1u
/* Actual queued CHARIN2077/060101; compose this resident mapping with basic
 * and arithmetic mappings using arithmetic-first wrapper storage. VERB/NOUN
 * execute complete CHARIN and real ENDOFJOB; other key bodies stop explicitly
 * at the native unsupported boundary without releasing the admitted core. */
extern const exec_context_mapping dsky_keyboard_mappings[DSKY_KEYBOARD_MAPPING_COUNT];
/* Actual idle NUDIRECT/SUPDXCHZ startup for admitted core0 CHARIN. Source
 * semantic entry requires activity A=2 and IRQ enabled; emits real activity
 * channel011 event, materializes source dispatch register debris and calls the
 * native Executive start. Other queued-slot idle routes remain separate. */
enum exec_status dsky_keyboard_start(dsky_keyboard_binding *);
/* Foreground safe-point delivery: hardware frame stores ZRUPT and the literal
 * source instruction word as BRUPT, then complete native key IRQ/RESUME. The
 * word is never decoded; running C continuations remain explicit mappings. */
enum exec_status dsky_keyboard_interrupt(dsky_keyboard_binding *,uint16_t channel15,uint16_t source_instruction);
/* Actual idle ADVAN3214 +0 branch through CAF TWO/NUDIRECT, after IRQ resume.
 * Executes no AGC instruction data; supports admitted core0 CHARIN only. */
enum exec_status dsky_keyboard_idle(dsky_keyboard_binding *);
#endif
