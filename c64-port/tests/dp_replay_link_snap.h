/* SPDX-License-Identifier: MIT */
/* A fixed snapshot for tools/dp_replay_link.py, which only links the replay
 * harness: tools/flight_host_run.c's FLIGHT_SNAPSHOT format with every field
 * it writes, the values placeholders (nothing runs). */
#define SNAP_CS 1000u
static const uint16_t snap_words[2048] SNAP_DATA={0u};
#define SNAP_IO(io) (io).superbank=64u;(io).interrupt_enabled=1u;
#define SNAP_HW(h) (h).channel11=4096u;(h).in_interrupt=0u;(h).channel5=0u;(h).channel6=0u;(h).channel13=0u;(h).channel15=0u;(h).channel10=0u;(h).channel32=32767u;(h).channel12=0u;(h).channel14=0u;(h).channel30=15593u;(h).channel33=32187u;(h).channel31=24575u;(h).channel3=0u;(h).channel4=0u;(h).radar_gate=0u;
#define SNAP_FLIGHT(h) (h).superbank=64u;(h).interrupt_enabled=1u;(h).channel16=0u;(h).channel34=32767u;(h).channel35=32767u;(h).trap31a=0u;(h).trap31b=0u;(h).trap32=0u;(h).executive_alarm=0u;(h).waitlist_alarm=0u;(h).warning=0u;
#define SNAP_CDU(c) (c).owed[0]=0;(c).owed[1]=0;(c).owed[2]=0;(c).owed[3]=0;(c).owed[4]=0;(c).elapsed=0u;(c).active=0u;
