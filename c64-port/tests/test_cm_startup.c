/* SPDX-License-Identifier: MIT */
#include "cm_startup_check.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static apollo_overlay_state image,before;
static cm_native_io io,saved_io;
static cm_startup_ports ports,saved_ports;
int main(void) {
 assert(cm_startup_check(&image,&io,&ports)==0);
 cm_startup_prepare(&image,&io,&ports,0);image.words[6]=012002;before=image;saved_io=io;saved_ports=ports;
 assert(cm_startup_begin(&image,&io,&ports,CM_STARTUP_DOFSTART)==CM_STARTUP_OUT_OF_SCOPE);
 assert(memcmp(&image,&before,sizeof image)==0 && memcmp(&io,&saved_io,sizeof io)==0 && memcmp(&ports,&saved_ports,sizeof ports)==0);
 cm_startup_prepare(&image,&io,&ports,0);before=image;
 assert(cm_startup_begin(&image,&io,&ports,(enum cm_startup_entry)9)==CM_STARTUP_OUT_OF_SCOPE);
 assert(memcmp(&image,&before,sizeof image)==0);
 puts("PASS:640originalCMstartup/cleanup whole-image states; all2047image words+15I/O values;5fullsamples andinvalid-entry/bank preservation");return 0;
}
