/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "flight_io.h"
/* A routine may write a channel straight to agc_hardware (p->hardware: the
 * CM T4RUPT's WOR and WAND on channels 11, 12 and 14, KEYRUPT's DSALMOUT)
 * while its table's record holds a copy of that channel. flight_io_leave
 * must not undo that write with the record's unchanged copy. */
#ifdef __mos__
/* The C64: on the CM, rk_device (the cm_native_io record) holds channels
 * 11-14 and the resident merges them with rk_hardware's after each step
 * (platform/c64/resident_flight.c rk_channels_merge), so a module leaves
 * rk_hardware's 11-14 alone (CM_PUT). No routine of an LM table on a record
 * with channels writes p->hardware for a channel its record holds, and the
 * modules have no room to spare: plain copies (PUT). */
#define PUT(field, value)    (hw->field = (agc_word)(value))
#define CM_PUT(field, value) ((void)(value))
#define ENTRY(hw)            ((void)(hw))
#else
/* The host and x86: the channels as flight_io_enter found them; a record's
 * channel goes back only when the record's routines changed it. One entry:
 * enter and leave must not nest (nothing does today). */
static agc_hardware entry;
#define PUT(field, value)                                                                          \
    do {                                                                                           \
        if ((agc_word)(value) != entry.field) hw->field = (agc_word)(value);                       \
    } while (0)
#define CM_PUT    PUT
#define ENTRY(hw) (entry = *(hw))
#endif
static void p66_in(p66_bias_io *b, const flight_io *io)
{
    b->superbank = io->superbank;
    b->interrupt_enabled = io->interrupt_enabled;
}
static void p66_out(const p66_bias_io *b, flight_io *io)
{
    io->superbank = b->superbank;
    io->interrupt_enabled = b->interrupt_enabled;
}
static void cm_in(cm_native_io *c, const flight_io *io, const agc_hardware *hw)
{
    c->channel11 = hw->channel11;
    c->channel12 = hw->channel12;
    c->channel13 = hw->channel13;
    c->channel14 = hw->channel14;
    c->main_keys = hw->channel15;
    c->navigation_keys = io->channel16;
    c->superbank = io->superbank;
    c->interrupt_enabled = io->interrupt_enabled;
    c->trap31a = io->trap31a;
    c->trap31b = io->trap31b;
    c->trap32 = io->trap32;
    c->executive_alarm = io->executive_alarm;
    c->waitlist_alarm = io->waitlist_alarm;
}
static void cm_out(const cm_native_io *c, flight_io *io, agc_hardware *hw)
{
    CM_PUT(channel11, c->channel11);
    CM_PUT(channel12, c->channel12);
    CM_PUT(channel13, c->channel13);
    CM_PUT(channel14, c->channel14);
    PUT(channel15, c->main_keys);
    io->channel16 = c->navigation_keys;
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    io->trap31a = (uint16_t)c->trap31a;
    io->trap31b = (uint16_t)c->trap31b;
    io->trap32 = (uint16_t)c->trap32;
    io->executive_alarm = c->executive_alarm;
    io->waitlist_alarm = c->waitlist_alarm;
}
static void inputs(agc_input_channels *in, const flight_io *io, const agc_hardware *hw)
{
    in->channel3 = hw->channel3;
    in->channel4 = hw->channel4;
    in->channel30 = hw->channel30;
    in->channel31 = hw->channel31;
    in->channel32 = hw->channel32;
    in->channel33 = hw->channel33;
    in->channel16 = io->channel16;
}
void *flight_io_enter(enum flight_io_kind kind, const flight_io *io, const agc_hardware *hw,
                      flight_io_buffer *b)
{
    memset(b, 0, sizeof *b);
    ENTRY(hw);
    switch (kind) {
    case FLIGHT_IO_P66: p66_in(&b->p66, io); break;
    case FLIGHT_IO_CM_NATIVE: cm_in(&b->cm, io, hw); break;
    case FLIGHT_IO_LM_CHANNEL:
        p66_in(&b->lm_channel.io, io);
        inputs(&b->lm_channel.in, io, hw);
        break;
    case FLIGHT_IO_CM_CHANNEL:
        cm_in(&b->cm_channel.io, io, hw);
        inputs(&b->cm_channel.in, io, hw);
        break;
    case FLIGHT_IO_LM_FRESH: {
        lm_fresh_io *x = &b->lm_fresh;
        p66_in(&x->control, io);
        x->channel5 = hw->channel5;
        x->channel6 = hw->channel6;
        x->channel12 = hw->channel12;
        x->channel13 = hw->channel13;
        x->channel14 = hw->channel14;
        x->channel15 = hw->channel15;
        x->channel16 = io->channel16;
        x->channel33 = hw->channel33;
        x->trap31a = io->trap31a;
        x->trap31b = io->trap31b;
        x->trap32 = io->trap32;
        break;
    }
    case FLIGHT_IO_LM_SV: {
        lm_sv_io *x = &b->lm_sv;
        p66_in(&x->control, io);
        x->chan11 = hw->channel11;
        x->chan12 = hw->channel12;
        x->chan32 = hw->channel32;
        x->chan33 = hw->channel33;
        x->undo = io->undo;
        break;
    }
    case FLIGHT_IO_LM_LLD: {
        lm_lld_io *x = &b->lm_lld;
        p66_in(&x->control, io);
        x->chan12 = hw->channel12;
        x->chan33 = hw->channel33;
        x->undo = io->undo;
        break;
    }
    case FLIGHT_IO_LM_IPT: {
        lm_ipt_io *x = &b->lm_ipt;
        p66_in(&x->control, io);
        x->hiscalar = hw->channel3;
        x->loscalar = hw->channel4;
        x->undo = io->undo;
        break;
    }
    case FLIGHT_IO_LM_BB: {
        lm_bb_io *x = &b->lm_bb;
        p66_in(&x->control, io);
        x->chan11 = hw->channel11;
        x->chan14 = hw->channel14;
        x->chan30 = hw->channel30;
        x->undo = io->undo;
        break;
    }
    case FLIGHT_IO_LM_AGS: {
        lm_ags_io *x = &b->lm_ags;
        p66_in(&x->control, io);
        x->undo = io->undo;
        break;
    }
    case FLIGHT_IO_LM_IMU: {
        lm_imu_io *x = &b->lm_imu;
        p66_in(&x->control, io);
        x->channel12 = hw->channel12;
        x->channel13 = hw->channel13;
        x->channel14 = hw->channel14;
        break;
    }
    case FLIGHT_IO_LM_DNTM: {
        lm_dntm_io *x = &b->lm_dntm;
        p66_in(&x->control, io);
        x->channel11 = hw->channel11;
        x->channel12 = hw->channel12;
        x->channel13 = hw->channel13;
        x->channel14 = hw->channel14;
        x->channel30 = hw->channel30;
        x->channel31 = hw->channel31;
        x->channel32 = hw->channel32;
        x->channel33 = hw->channel33;
        x->channel34 = io->channel34;
        x->channel35 = io->channel35;
        x->trap31a = io->trap31a;
        x->trap31b = io->trap31b;
        x->trap32 = io->trap32;
        break;
    }
    case FLIGHT_IO_CM_FRESH: {
        cm_fresh_io *x = &b->cm_fresh;
        cm_in(&x->control, io, hw);
        x->channel5 = hw->channel5;
        x->channel6 = hw->channel6;
        x->channel33 = hw->channel33;
        x->warning = io->warning;
        break;
    }
    case FLIGHT_IO_CM_DNTM: {
        cm_dntm_io *x = &b->cm_dntm;
        cm_in(&x->io, io, hw);
        x->channel30 = hw->channel30;
        x->channel31 = hw->channel31;
        x->channel32 = hw->channel32;
        x->channel33 = hw->channel33;
        x->channel34 = io->channel34;
        x->channel35 = io->channel35;
        break;
    }
    default: break;
    }
    return b;
}
void flight_io_leave(enum flight_io_kind kind, flight_io *io, agc_hardware *hw,
                     const flight_io_buffer *b)
{
    switch (kind) {
    case FLIGHT_IO_P66: p66_out(&b->p66, io); break;
    case FLIGHT_IO_CM_NATIVE: cm_out(&b->cm, io, hw); break;
    case FLIGHT_IO_LM_CHANNEL: p66_out(&b->lm_channel.io, io); break;
    case FLIGHT_IO_CM_CHANNEL: cm_out(&b->cm_channel.io, io, hw); break;
    case FLIGHT_IO_LM_FRESH: {
        const lm_fresh_io *x = &b->lm_fresh;
        p66_out(&x->control, io);
        PUT(channel5, x->channel5);
        PUT(channel6, x->channel6);
        PUT(channel12, x->channel12);
        PUT(channel13, x->channel13);
        PUT(channel14, x->channel14);
        PUT(channel15, x->channel15);
        io->channel16 = x->channel16;
        PUT(channel33, x->channel33);
        io->trap31a = x->trap31a;
        io->trap31b = x->trap31b;
        io->trap32 = x->trap32;
        break;
    }
    case FLIGHT_IO_LM_SV: {
        const lm_sv_io *x = &b->lm_sv;
        p66_out(&x->control, io);
        PUT(channel11, x->chan11);
        PUT(channel12, x->chan12);
        PUT(channel32, x->chan32);
        PUT(channel33, x->chan33);
        break;
    }
    case FLIGHT_IO_LM_LLD: {
        const lm_lld_io *x = &b->lm_lld;
        p66_out(&x->control, io);
        PUT(channel12, x->chan12);
        PUT(channel33, x->chan33);
        break;
    }
    case FLIGHT_IO_LM_IPT: {
        const lm_ipt_io *x = &b->lm_ipt;
        p66_out(&x->control, io);
        PUT(channel3, x->hiscalar);
        PUT(channel4, x->loscalar);
        break;
    }
    case FLIGHT_IO_LM_BB: {
        const lm_bb_io *x = &b->lm_bb;
        p66_out(&x->control, io);
        PUT(channel11, x->chan11);
        PUT(channel14, x->chan14);
        PUT(channel30, x->chan30);
        break;
    }
    case FLIGHT_IO_LM_AGS: p66_out(&b->lm_ags.control, io); break;
    case FLIGHT_IO_LM_IMU: {
        const lm_imu_io *x = &b->lm_imu;
        p66_out(&x->control, io);
        PUT(channel12, x->channel12);
        PUT(channel13, x->channel13);
        PUT(channel14, x->channel14);
        break;
    }
    case FLIGHT_IO_LM_DNTM: {
        const lm_dntm_io *x = &b->lm_dntm;
        p66_out(&x->control, io);
        PUT(channel11, x->channel11);
        PUT(channel12, x->channel12);
        PUT(channel13, x->channel13);
        PUT(channel14, x->channel14);
        PUT(channel30, x->channel30);
        PUT(channel31, x->channel31);
        PUT(channel32, x->channel32);
        PUT(channel33, x->channel33);
        io->channel34 = x->channel34;
        io->channel35 = x->channel35;
        io->trap31a = x->trap31a;
        io->trap31b = x->trap31b;
        io->trap32 = x->trap32;
        break;
    }
    case FLIGHT_IO_CM_FRESH: {
        const cm_fresh_io *x = &b->cm_fresh;
        cm_out(&x->control, io, hw);
        PUT(channel5, x->channel5);
        PUT(channel6, x->channel6);
        PUT(channel33, x->channel33);
        io->warning = x->warning;
        break;
    }
    case FLIGHT_IO_CM_DNTM: {
        const cm_dntm_io *x = &b->cm_dntm;
        cm_out(&x->io, io, hw);
        PUT(channel30, x->channel30);
        PUT(channel31, x->channel31);
        PUT(channel32, x->channel32);
        PUT(channel33, x->channel33);
        io->channel34 = x->channel34;
        io->channel35 = x->channel35;
        break;
    }
    default: break;
    }
}
