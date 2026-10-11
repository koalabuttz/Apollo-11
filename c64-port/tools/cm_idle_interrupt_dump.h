/* SPDX-License-Identifier: MIT; host-only complete source image. */
#include "cm_alarm2_image_dump.h"
static void cm_idle_interrupt_dump(uint32_t n,uint16_t status,const apollo_overlay_state *s,const cm_native_io *io,const cm_idle_interrupt_state *irq){unsigned char v[4]={irq->in_isr,0,irq->pending_idle,0};cm_alarm2_image_dump(n,status,s,io);if(fwrite(v,1,4,stdout)!=4)exit(2);}
