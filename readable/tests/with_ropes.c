/* SPDX-License-Identifier: MIT */
/* main() for a faithful port's test that runs readable interpreter code
 * (make stateup-cases): the readable interpreter reads the program's rope
 * (its fixed constants, as the AGC's interpreter does), which the port's
 * own tests never load. This loads both missions' ropes, named on the
 * command line, then runs the test, built with its main() renamed
 * port_main(). */
#include <stdio.h>
#include "rope_image.h"

int port_main(void);

int main(int argc, char **argv)
{
    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    return port_main();
}
