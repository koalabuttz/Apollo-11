/* SPDX-License-Identifier: MIT */
#ifndef P66_INITIALIZATION_H
#define P66_INITIALIZATION_H
#include "agc_word.h"
enum p66_control_word {P66_OMEGAPD,P66_OMEGAQD,P66_OMEGARD,P66_DELCDUX,P66_DELCDUY,P66_DELCDUZ,P66_DELPEROR,P66_DELQEROR,P66_DELREROR,P66_CONTROL_WORDS};
typedef struct {
 agc_word control[P66_CONTROL_WORDS],phase3_address[2];
 agc_register a,l;
 agc_word q,bbank,superbank,interrupt_enabled;
} p66_basic_initialization;
enum p66_init_status {P66_INIT_OK=0,P66_INIT_OUT_OF_SCOPE=1};
/* Original STOPRATE through normal caller return: clear all nine DAP rate/
 * error interface words, restore caller EBANK, preserve producer exclusion.
 * Q is an ordinary basic return address0100..07777. No display placeholder. */
enum p66_init_status p66_stoprate(p66_basic_initialization *);
/* Specialized FASTCHNG only. The original performs a single PHSNAME3 restart-location word
 * store, retaining its BBCON companion for the ensuing restart protocol. Caller
 * receives original register debris and physical bank unchanged. */
enum p66_init_status p66_fastchng(p66_basic_initialization *);
#endif
