/* SPDX-License-Identifier: MIT */
#ifndef EDGE_COVER_H
#define EDGE_COVER_H
#include <stdio.h>
#include <stdint.h>
/* Host-only branch-outcome recorder for choosing C64 phase rows. Sources
 * compiled with clang -fsanitize-coverage=edge,trace-pc-guard get one guard
 * per control-flow edge (critical edges split, so every branch outcome has
 * its own guard). A driver calls edge_cover_mark after each phase step;
 * steps whose set of edges was not seen before are printed as
 * "tag... | e1 e2 ...", so the generator can pick a small set of steps that
 * runs every edge the whole host stream runs. */
void edge_cover_mark(FILE *out,const char *tag);
unsigned edge_cover_edges(void);
#endif
