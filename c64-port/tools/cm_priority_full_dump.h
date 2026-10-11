/* SPDX-License-Identifier: MIT; host-only complete image and selected tuple. */
#include "cm_alarm2_image_dump.h"
static void cm_priority_full_dump(uint32_t n,const apollo_overlay_state *s,const cm_native_io *io,const exec_transfer *next){agc_word v[3]={next->interpretive,next->address.location,next->address.bank};unsigned char b[6];unsigned i;cm_alarm2_image_dump(n,EXEC_OK,s,io);for(i=0;i<3u;++i){b[2u*i]=(unsigned char)v[i];b[2u*i+1u]=(unsigned char)(v[i]>>8);}if(fwrite(b,1,6,stdout)!=6)exit(2);}
