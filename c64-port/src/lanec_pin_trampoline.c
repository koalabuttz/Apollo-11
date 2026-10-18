/* SPDX-License-Identifier: MIT */
/* The Pinball transfer-ABI trampoline (include/lanec_pin_routines.h): a
 * routine's claiming modules run in turn on the device record's channels.
 * The routine tables (src/lanec_pin_routines.c and the keypress groups'
 * src/lanec_pin_keys_*_routines.c) share it. */
#include <string.h>
#include "lanec_pin_routines.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

static lanec_io io;                 /* the trampoline runs one routine at a time */
#if !defined(LANEC_PIN_C64) && defined(__mos__)
/* A flight module keeps the state each claiming module starts from in the
 * REU, not in its 16 KB slot: the last 4 KB below the CM rope image, above
 * the LM one (include/rope_image.h: 36 K words, two bytes each, from
 * 0x1c0000 to 0x1d2000), which neither rope nor the module store uses. */
#include "overlay_reu.h"
#define LANEC_PIN_REU_SAVE 0x1df000ul
#define LANEC_PIN_SAVE(s) ((void)apollo_reu_transfer(APOLLO_REU_STASH, (s), LANEC_PIN_REU_SAVE, sizeof *(s)))
#define LANEC_PIN_RESTORE(s) ((void)apollo_reu_transfer(APOLLO_REU_FETCH, (s), LANEC_PIN_REU_SAVE, sizeof *(s)))
#elif !defined(LANEC_PIN_C64)
static apollo_overlay_state entry;  /* the state each claiming module starts from */
#define LANEC_PIN_SAVE(s) (entry = *(s))
#define LANEC_PIN_RESTORE(s) (*(s) = entry)
#else   /* one module, which leaves a refused state unchanged; a result the record
         * cannot carry is refused changed, and the caller makes the case again */
#define LANEC_PIN_SAVE(s) ((void)0)
#define LANEC_PIN_RESTORE(s) ((void)0)
#endif

const uint16_t lanec_pin_cells[LANEC_PIN_CELLS] = { 072u, 0114u, 0115u, 0116u, 0117u, 0124u, 0125u, 0136u, 0144u, 0154u, 0156u,
                                                     0162u, 01013u };

/* A module that ends at a register or a return cell holding a TC (not to
 * itself, and not RELINT, INHINT or EXTEND) or a TCF into fixed memory:
 * execute that instruction. */
static void cell_return(apollo_overlay_state *s)
{
    uint16_t z = s->words[5], i, cell = z < 8u, w;
    for (i = 0; i < LANEC_PIN_CELLS; ++i)
        if (lanec_pin_cells[i] == z) cell = 1;
    if (!cell) return;
    w = s->words[z];
    if (w <= 07777u && w != 3u && w != 4u && w != 6u && w != z) {   /* TC */
        s->words[5] = w;
        s->words[2] = (uint16_t)(z + 1u);
    } else if (w >= 012000u && w <= 017777u) {  /* TCF */
        s->words[5] = (uint16_t)(w & 07777u);
    }
}

/* A channel the device record does not carry was written (CHANLOAD's
 * WRITE to any channel): the routine cannot give that result. */
static int unrepresented(uint16_t last)
{
    uint16_t c;
    for (c = 0; c < LANEC_CHANNELS; ++c)
        if (io.channel[c] && c != 1u && c != 2u && c != 7u && (c < 011u || c > last)) return 1;
    return 0;
}

/* The program's fixed memory for the modules that read the rope as data
 * (lanec_fixed_read); a program without fixed_data keeps the harness's. */
static const agc_native_program *program;
static bool program_fixed(enum lanec_mission mission, uint16_t physical, agc_word *word)
{
    (void)mission;
    return program->fixed_data(program, physical, word);
}
static void use_program(const agc_native_program *p)
{
    if (p->fixed_data) { program = p; lanec_fixed_read = program_fixed; }
}

#ifdef LANEC_PIN_HAS_LM
enum exec_status lanec_pin_run_lm(const agc_native_program *p, apollo_overlay_state *s, void *v, const lanec_pin_dispatch *f)
{
    lm_dntm_io *x = v;
    if (!LANEC_MISSION_LINKED(LANEC_LM) || !p || !s || !x || !p->hardware || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    use_program(p);
    LANEC_PIN_SAVE(s);
    for (; *f; ++f) {
        memset(&io, 0, sizeof io);
        io.superbank = x->control.superbank; io.interrupt_enabled = x->control.interrupt_enabled;
        io.channel[011] = p->hardware->channel11;
        io.channel[012] = x->channel12; io.channel[013] = x->channel13; io.channel[014] = x->channel14;
        if ((*f)(s, &io, LANEC_LM) == LANEC_OK && !unrepresented(014u)) {
            x->control.superbank = io.superbank; x->control.interrupt_enabled = io.interrupt_enabled;
            p->hardware->channel11 = x->channel11 = io.channel[011];
            x->channel12 = io.channel[012]; x->channel13 = io.channel[013]; x->channel14 = io.channel[014];
            cell_return(s);
            return EXEC_OK;
        }
        LANEC_PIN_RESTORE(s);
    }
    return EXEC_OUT_OF_SCOPE;
}

#endif

#ifdef LANEC_PIN_HAS_CM
enum exec_status lanec_pin_run_cm(const agc_native_program *p, apollo_overlay_state *s, void *v, const lanec_pin_dispatch *f)
{
    cm_dntm_io *x = v;
    if (!LANEC_MISSION_LINKED(LANEC_CM) || !p || !s || !x || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    use_program(p);
    LANEC_PIN_SAVE(s);
    for (; *f; ++f) {
        memset(&io, 0, sizeof io);
        io.superbank = x->io.superbank; io.interrupt_enabled = x->io.interrupt_enabled;
        io.channel[011] = x->io.channel11; io.channel[012] = x->io.channel12;
        io.channel[013] = x->io.channel13; io.channel[014] = x->io.channel14;
        io.channel[015] = x->io.main_keys; io.channel[016] = x->io.navigation_keys;
        if ((*f)(s, &io, LANEC_CM) == LANEC_OK && !unrepresented(016u)) {
            x->io.superbank = io.superbank; x->io.interrupt_enabled = io.interrupt_enabled;
            x->io.channel11 = io.channel[011]; x->io.channel12 = io.channel[012];
            x->io.channel13 = io.channel[013]; x->io.channel14 = io.channel[014];
            x->io.main_keys = io.channel[015]; x->io.navigation_keys = io.channel[016];
            cell_return(s);
            return EXEC_OK;
        }
        LANEC_PIN_RESTORE(s);
    }
    return EXEC_OUT_OF_SCOPE;
}
#endif
