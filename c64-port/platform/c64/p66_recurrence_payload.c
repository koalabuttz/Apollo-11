/* SPDX-License-Identifier: MIT */
#include "p66_recurrence_payload.h"
#include "p66_length_return_control.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){p66_recurrence_arguments *a=(p66_recurrence_arguments *)(uintptr_t)pointer;if(!a||!a->binding||a->binding->application.display_job.descent.basic.arithmetic.image!=image)return 0xffff;
#if P66_RECURRENCE_SERVICE==87
 return a->ticks==193u?p66_feedback_store_return_control(image):p66_length_return_control(image,a->ticks);
#else
 return p66_recurrence_entry(a->binding,a->next,P66_RECURRENCE_SERVICE,a->ticks,a->consumed);
#endif
}
