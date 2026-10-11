/* SPDX-License-Identifier: MIT */
#ifndef RTB_ANGLES_H
#define RTB_ANGLES_H
#include "p66_bias.h"
#include "agc_multiword.h"
enum rtb_mission {RTB_LM=0,RTB_CM=1};
enum rtb_angle_entry {RTB_ONE_TO_TWO_SUB=0,RTB_ONE_TO_TWO=1,RTB_VECTOR_ONE_TO_TWO=2,RTB_TWO_VECTOR_ONE_TO_TWO=3,RTB_CDU_LOGIC=4,RTB_CDU_INCREMENT=5,RTB_INCREMENT_CDUS=6,RTB_LOAD_TIME=7,RTB_READ_PIPS=8};
enum rtb_status {RTB_OK=0,RTB_OUT_OF_SCOPE=1};
/* Source RTB angle/time/PIPA entries only; caller owns producers/canonical
 * nonnull image/io with ordinary15-bit words/raw16-bit A/L, source bank10 LM or22 CM,
 * coherent EB/BB, REGZERO0 and SUPER bits0160. MODE may be any ordinaryword.
 * Direct 1TO2SUB/CDUINC requires fixed Q02000..07777; returns PC=Q/Q3.
 * CDUINC requires normalized ordinary A and BUF subaddress040..01777,
 * with banked target resolved through incomingEB, physical target>=040. All ordinary target words,
 * including source scratch aliases, retain literal source sequencing.
 * Other entries finish before DANZIG (LM06061/CM06030), preserving pending
 * LOC/EDOP and bank controls. READPIPS/INCRCDUS are CM-only; READPIPS reads
 * source PIPA words under INHINT then returns IRQ1 after source RELINT.
 * No hardware gyro service or complete RTB/RT8 containing-file claim here. */
enum rtb_status rtb_angles(apollo_overlay_state *,p66_bias_io *,enum rtb_mission,enum rtb_angle_entry);
/* Qualified containing LOCTHETA only (CM); no other RTB constants claimed. */
uint8_t rtb_angles_constant_read(enum rtb_mission,uint16_t physical_fixed_word,uint16_t *);
#endif
