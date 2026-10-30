/* SPDX-License-Identifier: MIT */
/* The readable Executive and WAITLIST as routines of the flight program.
 *
 * The x86_64 flight program (c64-port/docs/X86-FLIGHT.md) runs the AGC
 * program as native routines keyed by the address they start at. This file
 * offers the readable routines under the same keys and table names as the
 * faithful port's resident kernel (c64-port/src/lm_kernel.c and
 * cm_kernel.c), so a program links one or the other: the readable build
 * (readable/Makefile) links these in place of those.
 *
 * Its job is plumbing only: point the machine (src/agc/agc.h) at the
 * program's state, read the inline operands a TC passes (the 2CADR after a
 * TC NOVAC) from the program's operand table, and call the routine. */
#include <stddef.h>
#include "resident_kernel.h"
#if LUMINARY099
#include "p66_bias.h"
#else
#include "cm_native_image.h"
#endif
#include "../executive/executive.h"
#include "../waitlist/waitlist.h"
#include "../agc/names.h"

#if LUMINARY099
typedef p66_bias_io mission_io;
#else
typedef cm_native_io mission_io;
#endif

#define T3RUPT_LEAD_IN 04014u /* the T3RUPT interrupt's address */
#define BANK01         02000u

/* The word the TC at Q-1 passes inline at q (the program's operand table). */
static bool inline_word(const agc_native_program *program, apollo_overlay_state *state, word q,
                        word *out)
{
    return agc_transfer_operand(program, state, *agc.superbank, q, out);
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static uint16_t no_channel;
    mission_io *io = device;
    agc_hardware *hardware;
    word entry, operand[2] = {0, 0};

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    hardware = program->hardware;
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
#if LUMINARY099
    agc.dsalmout = hardware ? &hardware->channel11 : NULL;
#else
    agc.dsalmout = &io->channel11;
#endif
    agc.in_interrupt = hardware ? &hardware->in_interrupt : NULL;
    entry = Z;

    /* Read the inline operands first, so that a call from a place the
     * operand table does not know changes nothing. */
    if (entry == K_NOVAC || entry == K_FINDVAC || entry == K_WAITLIST || entry == K_LONGCALL) {
        if (!inline_word(program, state, Q, &operand[0]) ||
            !inline_word(program, state, (word)(Q + 1u), &operand[1]))
            return EXEC_OUT_OF_SCOPE;
    } else if (entry == K_TWIDDLE || entry == K_FIXDELAY) {
        if (!inline_word(program, state, Q, &operand[0])) return EXEC_OUT_OF_SCOPE;
    }
    /* The job switches and the idle loop need channel 11; the end of a task
     * and T3RUPT need to know whether an interrupt is running. */
    if (!agc.dsalmout && (entry == K_JOBSLEEP || entry == K_ENDOFJOB || entry == K_PRIOCHNG ||
                          (FBANK == BANK01 && entry >= K_NUCHANG2 && entry <= K_NUDIRECT)))
        return EXEC_OUT_OF_SCOPE;
    if (!hardware && (entry == K_TASKOVER || entry == T3RUPT_LEAD_IN)) return EXEC_OUT_OF_SCOPE;
    if (!agc.dsalmout) agc.dsalmout = &no_channel; /* never written on these paths */

    switch (entry) {
    case K_NOVAC: novac(operand); break;
    case K_FINDVAC: findvac(operand); break;
    case K_SPVACIN: spvacin(); break;
    case K_SPVAC: spvac(); break;
    case K_CHANG1: chang1(); break;
    case K_CHANG2: chang2(); break;
    case K_JOBSLEEP: jobsleep(); break;
    case K_JOBWAKE: jobwake(); break;
    case K_PRIOCHNG: priochng(); break;
    case K_ENDOFJOB: endofjob(); break;
    case K_SUPDXCHZ: supdxchz(); break;
    case K_SUPDXCHZ + 1u: supdxchz_plus1(); break;
    case K_DUMMYJOB: dummyjob(); break;
    case K_DUMMYJOB + 2u: dummyjob_plus2(); break;
    case K_ADVAN: advan(); break;
    case K_NUCHANG2: nuchang2(); break;
    case K_TWIDDLE: twiddle(operand[0]); break;
    case K_WAITLIST: waitlist(operand); break;
    case K_DLY2 - 1u: dly2_minus1(); break;
    case K_FIXDELAY: fixdelay(operand[0]); break;
    case K_VARDELAY: vardelay(); break;
    case T3RUPT_LEAD_IN: t3rupt_lead_in(); break;
    case K_TASKOVER: taskover(); break;
    case K_SVCT3: svct3(); break;
    case K_SVCT3 + 015u: svct3_after_novac(); break;
    case K_SVCT3X + 2u: svct3x_after_fixdelay(); break;
    case K_LONGCALL: longcall(operand); break;
    case K_LNGCALL2: lngcall2(); break;
    case K_LONGCYCL: longcycl(); break;
    case K_LONGRTRN:
    case K_MUCHTIME + 4u: longrtrn(); break;
    case K_GETCADR: getcadr(); break;
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* Overlay seams (c64-port/include/resident_kernel.h): with RK_SEAMS the
 * program is told of each job switch and job end. */
void executive_exchanging_core_sets(word newjob)
{
#if RK_SEAMS
    rk_seams_exchange(newjob);
#else
    (void)newjob;
#endif
}

void executive_ending_job(void)
{
#if RK_SEAMS
    rk_seams_release();
#endif
}

/* ---- The routine tables: the faithful kernel's keys and names ---- */

/* Keys: physical addresses of fixed-fixed (04000-07777) and bank 01. */
#define FF(z) (uint16_t)(2048u + ((z) - 04000u))
#define B1(z) (uint16_t)(1024u + ((z) - 02000u))

#if LUMINARY099
static const agc_routine_entry services[] = {
    {B1(K_LNGCALL2), run}, {B1(K_LONGCYCL), run}, {B1(K_LONGRTRN), run}, {B1(K_MUCHTIME + 4u), run},
    {B1(K_GETCADR), run},  {FF(K_NOVAC), run},    {FF(K_FINDVAC), run},  {FF(K_TWIDDLE), run},
    {FF(K_WAITLIST), run}, {FF(K_LONGCALL), run}};
static const agc_routine_entry tasks[] = {{FF(T3RUPT_LEAD_IN), run},
                                          {FF(K_DLY2 - 1u), run},
                                          {FF(K_FIXDELAY), run},
                                          {FF(K_VARDELAY), run},
                                          {FF(K_TASKOVER), run}};
#else
static const agc_routine_entry services[] = {
    {B1(K_LNGCALL2), run},      {B1(K_LONGCYCL), run},  {B1(K_LONGRTRN), run},
    {B1(K_MUCHTIME + 4u), run}, {B1(K_GETCADR), run},   {FF(T3RUPT_LEAD_IN), run},
    {FF(K_NOVAC), run},         {FF(K_FINDVAC), run},   {FF(K_TWIDDLE), run},
    {FF(K_WAITLIST), run},      {FF(K_DLY2 - 1u), run}, {FF(K_FIXDELAY), run},
    {FF(K_VARDELAY), run},      {FF(K_TASKOVER), run},  {FF(K_LONGCALL), run}};
#endif
static const agc_routine_entry jobs[] = {
    {FF(K_SPVACIN), run},  {FF(K_SPVAC), run},   {FF(K_CHANG1), run},   {FF(K_CHANG2), run},
    {FF(K_JOBSLEEP), run}, {FF(K_JOBWAKE), run}, {FF(K_PRIOCHNG), run}, {FF(K_ENDOFJOB), run}};
static const agc_routine_entry idle[] = {
    {B1(K_NUCHANG2), run}, {B1(K_DUMMYJOB), run},     {B1(K_DUMMYJOB + 2u), run},
    {B1(K_ADVAN), run},    {FF(K_SUPDXCHZ), run},     {FF(K_SUPDXCHZ + 1u), run},
    {FF(K_SVCT3), run},    {FF(K_SVCT3 + 015u), run}, {FF(K_SVCT3X + 2u), run}};

#define COUNT(table) (uint16_t)(sizeof(table) / sizeof(table)[0])
#if LUMINARY099
const agc_routine_table lm_kernel_routines = {services, COUNT(services)};
const agc_routine_table lm_kernel_job_routines = {jobs, COUNT(jobs)};
const agc_routine_table lm_kernel_task_routines = {tasks, COUNT(tasks)};
const agc_routine_table lm_kernel_idle_routines = {idle, COUNT(idle)};
#else
const agc_routine_table cm_kernel_routines = {services, COUNT(services)};
const agc_routine_table cm_kernel_job_routines = {jobs, COUNT(jobs)};
const agc_routine_table cm_kernel_idle_routines = {idle, COUNT(idle)};
#endif
