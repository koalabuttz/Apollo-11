/* SPDX-License-Identifier: MIT
 * Source-defined interrupt register exchanges and native explicit return. */
#include "dsky_key_irq.h"
enum exec_status dsky_key_irq(apollo_overlay_state *s,dsky_key_irq_io *io,uint16_t channel15){uint16_t *w=s->words,old_bank=w[6],old_lrupt=w[011];agc_word arupt,lr,old_qrupt;enum exec_status status;
 if(channel15>077777 || w[3]>03400 || (w[3]&0377u)!=0 || w[4]>076000 || (w[4]&01777u)!=0 || w[6]!=(w[4]|(w[3]>>8)) || w[015]==0 || w[015]>010000 || io->control.superbank>0160 || (io->control.superbank&017u)!=0 || io->control.interrupt_enabled>1 || io->in_interrupt!=1 || (w[067]!=077777 && (w[067]>84 || w[067]%12u!=0)))return EXEC_OUT_OF_SCOPE;
 /* Fixed vector4024: DXCH ARUPT, CAF KEYRPTBB010100, XCH BBANK. */
 w[010]=agc_correct(w[0]);w[011]=agc_correct(w[1]);w[1]=agc_extend(old_lrupt);w[0]=agc_extend(old_bank);w[3]=0;w[4]=010000;w[6]=010000;w[5]=03274;
 status=dsky_keyrupt(s,&io->control,channel15);if(status!=EXEC_OK)return status;
 /* Source RESUME: QXCH QRUPT, CA BANKRUPT/XCH BBANK, DXCH ARUPT. */
 old_qrupt=w[012];w[0]=agc_extend(w[016]);w[012]=agc_correct(w[2]);w[2]=agc_extend(old_qrupt);
 /* The original QRUPT must be captured before QXCH stores current Q. */
 arupt=w[010];lr=w[011];w[010]=010000;w[011]=agc_correct(w[1]);w[0]=agc_extend(arupt);w[1]=agc_extend(lr);
 w[6]=(uint16_t)(w[016]&076007u);w[3]=(uint16_t)((w[6]&7u)<<8);w[4]=(uint16_t)(w[6]&076000u);w[5]=(uint16_t)(w[015]-1u);w[7]=0;io->control.interrupt_enabled=1;io->in_interrupt=0;return status;
}
