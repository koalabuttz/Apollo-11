/* SPDX-License-Identifier: MIT; input-only source constructors. */
#include "standard_lead_target.h"
#include "standard_lead_cases.h"
#include "standard_lead_bank_cases.h"
#include "standard_lead_interp_cases.h"
#include "standard_lead_wake_cases.h"
#include "standard_lead_cm_cases.h"
#include "standard_lead_resume_cases.h"
void standard_lead_target_input(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission,uint16_t kind,uint16_t n){
 enum standard_lead_phase phase;agc_word fixed;cm_native_io cmio;
 io->superbank=0160;io->interrupt_enabled=0;
 if(kind==0u)standard_lead_input(s,mission,n,&phase,&fixed);
 else if(kind==1u)standard_lead_bank_input(s,mission,n);
 else if(kind==2u)standard_lead_interp_input(s,mission,n);
 else if(kind==3u)standard_lead_wake_input(s,mission,n);
 else if(kind==4u){standard_lead_cm_input(s,&cmio,n);io->superbank=cmio.superbank;io->interrupt_enabled=cmio.interrupt_enabled;}
 else{standard_lead_resume_input(s,mission,n);io->interrupt_enabled=(agc_word)(n/512u);}
}
