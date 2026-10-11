/* SPDX-License-Identifier: MIT */
#include "dsky_alarm_recycle.h"
static enum p66_bias_status invoke_cycle(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_alarm_cycle_route *route,uint8_t cm){cm_native_io view={0};enum exec_status status;if(!cm)return dsky_alarm_cycle(s,io,route);view.superbank=io->control.superbank;view.interrupt_enabled=io->control.interrupt_enabled;view.channel11=io->channel11;status=cm_alarm_cycle(s,&view,&io->wrote_channel11,route);if(status!=EXEC_OK)return P66_BIAS_OUT_OF_SCOPE;io->control.superbank=view.superbank;io->control.interrupt_enabled=view.interrupt_enabled;io->channel11=view.channel11;return P66_BIAS_OK;}
static enum p66_bias_status recycle(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_recycle_route *route,uint8_t cm){uint16_t value=agc_negate(s->words[01041]),event;enum dsky_alarm_cycle_route cycle_route;enum dsky_nv_update_route updated;enum p66_bias_status status;
 if(route==0||s->words[5]!=(cm ? 04161u : 04145u)||(value>99u&&value!=077777u))return P66_BIAS_OUT_OF_SCOPE;
 status=invoke_cycle(s,io,&cycle_route,cm);if(status!=P66_BIAS_OK)return status;event=io->wrote_channel11;
 status=dsky_nv_update_general(s,&io->control,cm ? DSKY_NV_CM : DSKY_NV_LM,DSKY_NV_VERB_STORE,&updated);if(status!=P66_BIAS_OK)return status;
 if(updated==DSKY_NV_UPDATE_CCSHOLE){*route=DSKY_RECYCLE_CCS_HOLE;return P66_BIAS_OK;}
 status=invoke_cycle(s,io,&cycle_route,cm);if(status!=P66_BIAS_OK)return status;io->wrote_channel11=event;*route=DSKY_RECYCLE_ENTER;return P66_BIAS_OK;
}
enum p66_bias_status dsky_alarm_recycle(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_recycle_route *route){return recycle(s,io,route,0);}
enum exec_status cm_alarm_recycle(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,enum dsky_recycle_route *route){dsky_charin_io view;enum p66_bias_status status;if(wrote==0)return EXEC_OUT_OF_SCOPE;view.control.superbank=io->superbank;view.control.interrupt_enabled=io->interrupt_enabled;view.channel11=io->channel11;view.wrote_channel11=0;status=recycle(s,&view,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=view.control.superbank;io->interrupt_enabled=view.control.interrupt_enabled;io->channel11=view.channel11;*wrote=view.wrote_channel11;return EXEC_OK;}
