/* SPDX-License-Identifier: MIT; immutable original source bindings. */
#ifndef SERVICE_ROUTINES_SOURCE_H
#define SERVICE_ROUTINES_SOURCE_H
#include <stdint.h>
typedef struct {
 uint16_t b5off;
 uint16_t bailout;
 uint16_t bailout1;
 uint16_t comflag;
 uint16_t debit;
 uint16_t delayjob;
 uint16_t delayloc;
 uint16_t delaynum;
 uint16_t delloop;
 uint16_t dly2;
 uint16_t downent2;
 uint16_t downflag;
 uint16_t endofjob;
 uint16_t gentran;
 uint16_t jobsleep;
 uint16_t jobwake;
 uint16_t ok2delay;
 uint16_t qplus1;
 uint16_t qplus2;
 uint16_t req;
 uint16_t taskover;
 uint16_t tcgetcad;
 uint16_t tcsleep;
 uint16_t tcsleep_word;
 uint16_t trfailof;
 uint16_t trfailon;
 uint16_t upent2;
 uint16_t upflag;
 uint16_t wakecad;
 uint16_t wakecad_word;
 uint16_t waker;
 uint16_t delay_count;
} service_routines_bindings;
static const service_routines_bindings service_routines_source[2]={
 {05563,05634,05716,05511,05522,03735,01326,04752,03740,05212,03554,05516,05155,05544,05133,05137,03752,06737,06741,00,05261,03761,03772,03757,00,00,03544,05504,03773,03765,03765,02},
 {05625,07755,00,05553,05564,03732,01141,06214,03735,05256,07740,05560,05217,05606,05175,05201,03746,06706,06710,05641,05324,03755,03766,03753,05631,05643,07722,05546,03767,03761,03761,03}
};
#endif
