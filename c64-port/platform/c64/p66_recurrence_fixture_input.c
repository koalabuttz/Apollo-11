/* SPDX-License-Identifier: MIT */
#include "p66_recurrence_fixture.h"
#include "p66_recurrence_live_inputs.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t number){p66_recurrence_fixture*f=P66_RECURRENCE_FIXTURE;unsigned i;if(number>=2u||!f->io||!f->channel14||!f->channel11)return 1;f->number=(uint8_t)number;f->stage=0;f->failures=0;*f->in_interrupt=0;f->io->superbank=prl_initial[number][0];f->io->interrupt_enabled=prl_initial[number][1];*f->channel14=prl_initial[number][2];*f->channel11=prl_initial[number][3];for(i=0;i<2048u;++i)image->words[i]=prl_initial[number][4u+i];return 0;}
