/* SPDX-License-Identifier: MIT
 * Thin resident Executive and Waitlist, Comanche055. */
#include "resident_kernel.h"
#include "cm_native_image.h"
#include "cm_kernel_labels.inc"
#define KERNEL_LM 0
#define KERNEL_CM 1
#define KIO cm_native_io
#define K_SUPER io->superbank
#define K_IE io->interrupt_enabled
#include "resident_kernel_body.inc"
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B1(z) (uint16_t)(1024u+((z)-02000u))
static const agc_routine_entry services[15] = {
    {B1(K_LNGCALL2), kernel},      {B1(K_LONGCYCL), kernel},  {B1(K_LONGRTRN), kernel},
    {B1(K_MUCHTIME + 4u), kernel}, {B1(K_GETCADR), kernel},   {FF(04014u), kernel},
    {FF(K_NOVAC), kernel},         {FF(K_FINDVAC), kernel},   {FF(K_TWIDDLE), kernel},
    {FF(K_WAITLIST), kernel},      {FF(K_DLY2 - 1u), kernel}, {FF(K_FIXDELAY), kernel},
    {FF(K_VARDELAY), kernel},      {FF(K_TASKOVER), kernel},  {FF(K_LONGCALL), kernel}};
static const agc_routine_entry jobs[8]={
 {FF(K_SPVACIN),kernel},{FF(K_SPVAC),kernel},{FF(K_CHANG1),kernel},{FF(K_CHANG2),kernel},
 {FF(K_JOBSLEEP),kernel},{FF(K_JOBWAKE),kernel},{FF(K_PRIOCHNG),kernel},{FF(K_ENDOFJOB),kernel}};
static const agc_routine_entry idle[9]={
 {B1(K_NUCHANG2),kernel},{B1(K_DUMMYJOB),kernel},{B1(K_DUMMYJOB+2u),kernel},{B1(K_ADVAN),kernel},
 {FF(K_SUPDXCHZ),kernel},{FF(K_SUPDXCHZ+1u),kernel},
 {FF(K_SVCT3),kernel},{FF(K_SVCT3+015u),kernel},{FF(K_SVCT3X+2u),kernel}};
const agc_routine_table cm_kernel_routines = {services, 15};
const agc_routine_table cm_kernel_job_routines={jobs,8};
const agc_routine_table cm_kernel_idle_routines={idle,9};
