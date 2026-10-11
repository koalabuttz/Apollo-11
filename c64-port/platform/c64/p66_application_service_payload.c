/* SPDX-License-Identifier: MIT */
#include "p66_application_service_payload.h"
#include "p66_direct_launch.h"
#include "p66_launch_phased.h"
#include "p66_mode_display.h"
#include "dsky_key_irq.h"
#include "dsky_charin_nv.h"
#include "dsky_charin_number.h"
#include "dsky_relay.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){
 p66_application_service_arguments *args=(p66_application_service_arguments *)(uintptr_t)pointer;
 if(!args||!args->control)return 0xffff;
#if P66_APPLICATION_SERVICE==50
 return (uint16_t)p66_launch_setup_phase(image,args->control);
#elif P66_APPLICATION_SERVICE==83
 return (uint16_t)p66_launch_bias_phase(image,args->control);
#elif P66_APPLICATION_SERVICE==51
 return (uint16_t)p66_mode_display(image,args->control);
#elif P66_APPLICATION_SERVICE==52
 return p66_direct_launch_entry(image,args->control,P66_DIRECT_VERTICAL_SERVICE);
#elif P66_APPLICATION_SERVICE==53
 {dsky_key_irq_io io;enum exec_status status;io.control=*args->control;io.in_interrupt=1;status=dsky_key_irq(image,&io,args->channel15);*args->control=io.control;return (uint16_t)status;}
#elif P66_APPLICATION_SERVICE==54||P66_APPLICATION_SERVICE==55
 {dsky_charin_io io;enum p66_bias_status status;if(!args->channel11)return 0xffff;io.control=*args->control;io.channel11=*args->channel11;io.wrote_channel11=0;
#if P66_APPLICATION_SERVICE==54
 status=dsky_charin_nv(image,&io);
#else
 status=dsky_charin_number(image,&io);
#endif
 *args->control=io.control;*args->channel11=io.channel11;return (uint16_t)status;}
#elif P66_APPLICATION_SERVICE==56
 {dsky_relay_io io;enum p66_bias_status status;io.control=*args->control;io.wrote_channel10=0;io.channel10=0;status=dsky_relay_drain(image,&io);*args->control=io.control;args->wrote_channel10=io.wrote_channel10;args->channel10=io.channel10;return (uint16_t)status;}
#else
#error Unknown P66_APPLICATION_SERVICE
#endif
}
