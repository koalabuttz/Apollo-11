/* SPDX-License-Identifier: MIT */
#include "native_overlay_dispatch.h"
#include "overlay_runtime.h"
#include <stdint.h>
typedef char scheduler_selector_holds_pointer[(sizeof(uintptr_t)==sizeof(uint16_t))?1:-1];
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t argument){exec_overlay_frame *frame=(exec_overlay_frame *)(uintptr_t)argument;(void)image;if(frame==0||frame->runtime==0)return 0xffff;
#if NATIVE_SCHEDULER_STAGE==1
 return(uint16_t)exec_native_prepare(frame->runtime,&frame->call);
#elif NATIVE_SCHEDULER_STAGE==2
 return(uint16_t)exec_native_finish(frame->runtime,frame->action,frame->resume);
#else
#error Unsupported native scheduler stage
#endif
}
