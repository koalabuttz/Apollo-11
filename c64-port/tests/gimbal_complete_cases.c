/* SPDX-License-Identifier: MIT
 * Input-only edge patterns; no native arithmetic or expected outputs. */
#include "gimbal_complete_cases.h"
#include "delcomp_cases.h"
enum apollo_fixed_mission gimbal_complete_mission(uint16_t n){return delcomp_mission(n);}
void gimbal_complete_prepare(apollo_overlay_state *s,gimbal_io *io,uint16_t n){uint16_t local=(uint16_t)(n%5120u),rate=(uint16_t)(2u*(local%4u)),am;
 delcomp_prepare(s,io,n);am=n<5120u?03335:03364;
 s->words[am]=s->words[0154];s->words[am+1u]=s->words[0155];
 s->words[n<5120u?01325:01130]=rate;s->words[0164]=n<5120u?((local&16u)?02746:02744):02737;
}
