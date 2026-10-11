/* SPDX-License-Identifier: MIT */
#include "cm_alarm_record_cases.h"
#include "cm_admission_complete_cases.h"
agc_word cm_alarm_record_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){static const agc_word pc[4]={05650,05764,05757,05655},corners[8]={0,077777,1,037777,040000,077776,01201,01103};unsigned c=n/4u;agc_word code=(agc_word)(c*371u&AGC_MASK);cm_admission_complete_input(s,io,n%CM_ADMISSION_COMPLETE_CASES);io->executive_alarm=0;s->words[5]=pc[n%4u];s->words[0]=agc_extend(code);s->words[1]=(agc_word)(c*137u);s->words[2]=0705;s->words[0705]=code;s->words[0375]=corners[c%8u];s->words[0376]=corners[(c/8u)%8u];s->words[0377]=corners[(c/64u)%8u];s->words[01036]=(agc_word)(c*713u&AGC_MASK);return code;}
