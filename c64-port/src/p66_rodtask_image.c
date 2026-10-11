/* SPDX-License-Identifier: MIT */
#include "p66_rodtask_image.h"
#include "p66_native.h"
enum exec_status p66_rodtask_image(apollo_overlay_state *image,p66_bias_io *io){executive e;p66_task_io task;enum exec_status status;uint16_t *w;
 if(!image||!io)return EXEC_OUT_OF_SCOPE;
 w=image->words;if(w[5]!=03263u||w[7]!=0||w[3]>03400u||(w[3]&0377u)||w[4]!=064000u||w[6]!=(uint16_t)(064000u|(w[3]>>8))||io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 p66_native_import_executive(&e,image);e.alarm=0;task.a=w[0];task.l=w[1];task.q=w[2];task.bbank=w[6];task.superbank=io->superbank;task.interrupt_enabled=io->interrupt_enabled;task.exectem1=w[061];task.exectem2=w[062];
 status=p66_rodtask(&e,&task);if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return status;
 p66_native_export_executive(image,&e);w[0]=task.a;w[1]=task.l;w[2]=task.q;w[3]=(uint16_t)((task.bbank&7u)*0400u);w[4]=(uint16_t)(task.bbank&076000u);w[6]=task.bbank;w[061]=task.exectem1;w[062]=task.exectem2;w[5]=status==EXEC_OK?05261u:05716u;w[7]=0;io->superbank=task.superbank;io->interrupt_enabled=task.interrupt_enabled;return status;
}
