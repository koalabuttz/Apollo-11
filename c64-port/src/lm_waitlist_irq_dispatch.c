/* SPDX-License-Identifier: MIT */
#include "lm_waitlist_irq_dispatch.h"
enum wl_status lm_waitlist_irq_dispatch(apollo_overlay_state *image,p66_bias_io *io,uint16_t in_interrupt,exec_address *next){p66_bias_io view;enum wl_status status;if(!io||io->interrupt_enabled!=1u)return WL_OUT_OF_SCOPE;view=*io;view.interrupt_enabled=0;status=lm_waitlist_dispatch(image,&view,in_interrupt,next);if(status==WL_OK){view.interrupt_enabled=1;*io=view;}return status;}
