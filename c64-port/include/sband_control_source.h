/* SPDX-License-Identifier: MIT; compile-time original source metadata. */
#ifndef SBAND_CONTROL_SOURCE_H
#define SBAND_CONTROL_SOURCE_H
#include <stdint.h>
typedef struct {uint16_t start,display,after_priority,terminate,proceed,recycle,blank,restart,reenter,bankcall,r02both,r02cadr,endext,gomarkfr,gomarkcadr,b5off,endofjob,blanket,priochng,delayjob,delaycadr,intpret,newops,prio5,prio4,vn;} sband_control_bindings;
static const sband_control_bindings sband_control_source[2]={
 {03606,03746,03754,03757,03760,03761,03762,03764,03766,04616,03254,011254,05472,02353,020353,05563,05155,05464,05146,03735,00,06037,06070,05000,04000,01463},
 {03652,03751,00,03760,03761,03762,03763,03765,03770,04662,03653,017653,05534,02600,020600,05625,05217,05526,05210,03732,01732,06006,06037,05000,04000,01463},
};
#endif
