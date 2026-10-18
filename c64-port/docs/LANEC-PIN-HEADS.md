# Pinball heads (lane C)

The Pinball modules of milestones lanec-pw-* each run a composite routine:
one C call that starts at a Pinball label and runs on through the routines
it calls (display, data requests, verb fan, ...) until the original reaches
its next registered point. On the host that is the shortest way to match
the original. On the C64 it is too big: a composite links every routine it
can reach, so one routine's closure can be 20-60 KB, over the 16 KB job
slot (tools/flight_store_build.py reports `lanec_pin_routines` "too big to
split").

A head family (`src/lanec_pin_head_<h>.c`) replaces composites at the points
it claims with the file's own words, run only from the point to the next
call or jump that leaves it. It leaves the state as the original leaves it
there (Z the callee, Q the word after the TC, A and erasables as the words
before the call set them), so the callee runs as its own registered routine
through the transfer engine (`agc_transfer_run`) and a head's closure is
small.

- Domain: `lanec_registers_valid`, the bank of the point and, where a word
  depends on it, the mission. Anything else returns `LANEC_OUT_OF_SCOPE`
  before any change, so the try-list falls through.
- Both missions in one source; `LANEC_ONE_MISSION` keeps a one-mission
  flight link small. A head whose words differ between missions names both
  addresses (include/lanec_symbols.h) or asserts at compile time that they
  match.
- `tools/lanec/routines.py pin` (cfg `heads`): at every point a head
  claims, it takes the place of the composites it names, at their position
  in the try-list. The script prints any point that still runs a replaced
  composite.

## Tests

Suite `pin-head-<h>` (`tests/lanec_pin_head_<h>_cases.c`, LANEC_PIN_HEADS
in tools/mk/lanec-pin-heads.mk): every point of the family, both missions.
An entry's states are the accepted cases of the composite the head replaces
at that entry; a return point's are those cases run through the Pinball
routines (`lanec_pin_chain`, tests/lanec_pin_transfer_cases.c) until they
reach it. A point no case reaches takes a filled state. The oracle runs the
original from the same state to where the head stops
(`lanec_pw_exit_stop`), and `tools/lanec/diff.py pin-head-<h> --refusals`
compares the complete state.

pin-transfer (the round trips through every registered routine, both
missions) covers the heads where they join the rest of Pinball.

## Families

| family | replaces | points |
|---|---|---|
| load | load_request, load_complete | ABCLOAD, ABLOAD, ALOAD, BLOAD, CLOAD calls and returns (bank 41); LOADLV, LOADLV +5 |
| lamp | lamp_lifecycle | VBTSTLTS (CM: and its BANKCALL return), TSTLTS1, TSTLTS2, TSTLTS3 (bank 41) |
| request | operator_request, operator_schedule | VBRQEXEC, VBRQWAIT (bank 41) |
| proceed | load_control | VBPROC, VBTERM, PROCKEY, VBRESEQ, RECALTST (bank 40) |
| nvsub | nv_admission, nv_consumer | NVSUB, NVMONOPT (fixed-fixed) |
| release | (none: before it, no routine took these points) | VBPROC +3..+5 (bank 40: the calls after KILMONON returns), RELDSP1 (fixed-fixed) |

Dropped: nv_update (nv_update_general accepts its states), decimal
(decimal_general and decimal_raw accept its states), Comanche's
decimal_general (cm_formatter and decimal_raw accept its states at
DSPDECWD; Luminary keeps it) and the six display composites the display
heads replace (docs/LANEC-PIN-HEADS-DISPLAY.md: decimal_consumer,
decimal_time_consumer, monitor_consumer, cm_dynamic_display,
cm_normal_display, cm_nv85, and minutes_output and Comanche's
cm_minutes_seconds, whose M/SOUT entry the minsec head also takes). pin-transfer and pin-decdsp3 complete as many
round trips without them, give or take a few cases.

A head's test cases must not leave for an address no caller returns to
(a TC Q with a made-up Q): routines.py takes every exit of every suite as
a place the original may go, and a made-up one turns other round trips
that stop there into refusals.

`dropped` (same cfg) removes composites from every try-list, not only at
head points: where the rest of a try-list or a head accepts every state the
composite reaches, the composite need not link on the C64 at all. Its
pw-* suite stays as a host test. A point only dropped modules claim fails
the script.
