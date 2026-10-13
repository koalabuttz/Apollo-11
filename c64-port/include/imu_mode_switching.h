/* SPDX-License-Identifier: MIT */
#ifndef IMU_MODE_SWITCHING_H
#define IMU_MODE_SWITCHING_H
#include "agc_transfer.h"
#include "p66_bias.h"
/* IMU_MODE_SWITCHING_ROUTINES (Luminary099 and Comanche055) as native
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md).
 *
 * The file spans four banks: ZEROICDU in fixed-fixed; the mode switching,
 * gyro torquing and stall code in bank 07; R02BOTH in bank 04 (Comanche
 * bank 07); P06/P05 in bank 37 (Comanche bank 26). Each routine is a
 * compiled translation of the source from its entry to the point where
 * control leaves the file: a TC or TCF to another file (WAITLIST, VARDELAY,
 * FIXDELAY, TASKOVER, ALARM, POODOO, BAILOUT1, IBNKCALL, BANKCALL, SWRETURN,
 * JOBSLEEP, JOBWAKE, MAKECADR, POSTJUMP, TPAGREE, SHORTMP, FINETIME +1,
 * UPFLAG, DOWNFLAG, PHASCHNG, VARALARM, GOTOPOOH), the caller's Q of a
 * subroutine entry (TC Q executes the TC that Q holds and leaves Q at 3),
 * TC MPAC +2 (SCALPREP's return, Q 0157), or an erasable TC that STRTGYR2's
 * computed jump reaches when LGYRO's step count is odd. Where the original
 * continues after such a call, the continuation is a routine of its own.
 * Local subroutines (CAGETEST, CAGETSTQ, CAGETSTJ, NOATTOFF, ZEROICDU,
 * SETCOARS, RNDREFDR, GSELECT, SCALPREP) are compiled into their callers
 * with the original Q effects.
 *
 * io is the mission's device record: lm_imu_io for Luminary (its first
 * member is the p66_bias_io other Luminary routines use), cm_native_io for
 * Comanche. INHINT and RELINT set interrupt_enabled; RAND, WAND and WOR use
 * channels 12, 13 and 14 there. READ DSALMOUT (ENDIMU) reads channel 11
 * from the program's agc_hardware; routines that can reach ENDIMU (2777,
 * the FIXDELAY continuation of COARS2, IMUFINED, NORESET, ENDIMU) refuse
 * when the program has none.
 *
 * Refused (EXEC_OUT_OF_SCOPE, image and io unchanged), states the original
 * program does not produce, where an index or computed jump would leave
 * the words the source addresses:
 *   - subroutine entries (ZEROICDU, SETCOARS, RNDREFDR, NOATTOFF) with Q
 *     outside 010..07777 (TC Q would not execute a TC);
 *   - BADEND and GOODEND with A other than 0, 1 or 2 (the MODECADR index);
 *     the MAKECADR continuation with RUPTREG2 above 2; the JOBWAKE
 *     continuation with LOCCTR above 01613 (INDEX LOCCTR ADS LOC);
 *   - the WAITLIST continuation of IMUPULSE with MPAC +5 low eight bits
 *     above 0372, and the TPAGREE continuation with MPAC +4 above 0376 or
 *     a further GYROAGRE pass indexing past 01776 (DCA 1400 would read
 *     fixed memory);
 *   - STRTGYRO with LGYRO negative or 040000 and above, or a GSELECT step
 *     whose LOW8 index reaches 0377;
 *   - SCALPREP's FINETIME continuation with MPAC +2 outside 010..07776;
 *   - the channel-13 writers (the PHASCHNG lead-in of P06 and POSTAND) with
 *     channel 13 bits 12-14 set (hardware never holds the trap enables). */
typedef struct {
 p66_bias_io control;
 agc_word channel12,channel13,channel14;
} lm_imu_io;
extern const agc_routine_table lm_imu_mode_switching_routines;
extern const agc_routine_table cm_imu_mode_switching_routines;
#define IMS_LM_ROUTINES 73u
#define IMS_CM_ROUTINES 73u
/* Physical keys of every routine, Luminary then Comanche in table order
 * (also in the C64 builds that link only part of the tables). */
extern const uint16_t ims_routine_keys[IMS_LM_ROUTINES+IMS_CM_ROUTINES];

/* Luminary099 entries and continuations (bank 07 unless noted). Comanche
 * addresses follow from the shared layout: ims_comanche_address(). */
#define IMS_ZEROICDU 05457u       /* fixed-fixed */
#define IMS_IMUZERO 02714u
#define IMS_MODEEXIT 02755u
#define IMS_IMUZERO2 02757u
#define IMS_IMUZERO3 02766u
#define IMS_IMUCOARS 03000u
#define IMS_COARS 03010u
#define IMS_COARS2 03040u
#define IMS_CA_ECE 03140u
#define IMS_SETCOARS 03144u
#define IMS_RNDREFDR 03175u
#define IMS_IMUFINE 03210u
#define IMS_IMUFINED 03232u
#define IMS_IFAILOK 03234u
#define IMS_PFAILOK 03254u
#define IMS_NOATTOFF 03266u
#define IMS_PIPUSE 03273u
#define IMS_PIPUSE1 03277u
#define IMS_PIPFREE 03310u
#define IMS_IMUPULSE 03323u
#define IMS_GWAKE 03374u
#define IMS_STRTGYRO 03405u
#define IMS_NORESET 03430u
#define IMS_GYROEXIT 03537u
#define IMS_8192AUG 03557u
#define IMS_ENDIMU 03631u
#define IMS_IMUBAD 03637u
#define IMS_BADEND 03662u
#define IMS_GOODEND 03665u
#define IMS_AOTSTALL 03712u
#define IMS_RADSTALL 03714u
#define IMS_IMUSTALL 03716u
#define IMS_GOMANUR 03750u
#define IMS_R02BOTH 03254u        /* bank 04 */
#define IMS_R02ZERO 03270u        /* bank 04 */
#define IMS_P06 03655u            /* bank 37 */
#define IMS_PRESTAND 03657u       /* bank 37 */
#define IMS_SCALPREP 03714u       /* bank 37 */
#define IMS_POSTAND 03734u        /* bank 37 */

/* The Comanche address of a Luminary address in the given Luminary bank
 * (02 for fixed-fixed). */
agc_word ims_comanche_address(unsigned lm_bank, agc_word lm_address);
#endif
