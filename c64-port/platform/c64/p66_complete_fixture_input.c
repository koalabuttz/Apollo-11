/* SPDX-License-Identifier: MIT */
#include "p66_complete_fixture.h"
#include "p66_complete_live_inputs.h"
uint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t number){p66_complete_fixture*f=P66_COMPLETE_FIXTURE;unsigned i;if(number>=2u||!f->io||!f->channel14||!f->channel11)return 1;f->number=(uint8_t)number;f->stage=0;f->failures=0;f->io->superbank=pcl_initial[number][0];f->io->interrupt_enabled=pcl_initial[number][1];*f->channel14=pcl_initial[number][2];*f->channel11=pcl_initial[number][3];for(i=0;i<2048u;++i)image->words[i]=pcl_initial[number][4u+i];return 0;}
