# Native mixed-noun component exchange

`dsky_mixed_noun` and `cm_mixed_noun` execute original MIXNOUN41,2221 through
actual VERBFAN41,2133 or GODSPALM41,2351. An unused noun alarms; verbs above6
bypass the component exchange. Display verbs0..6 and negative-zero verb execute
all three original descending component exchanges, with actual SFRUTMIX,
RIGHT5/LEFT5 editing-register effects, DPTEST, optional minor-word address
increment, SETEBANK and canonical source memory reads.

The two missions have a real DPTEST difference: routine13 is single-word
360-CDU in Luminary, but double-word DPFRACOT in Comanche. The native kernel
uses each original branch independently. It preserves exact EXITEM, DECOUNT,
NOUNTEM, NOUNADD, A/L/Q, bank and CYR/CYL debris. Source erasable reads include
ordinary memory, CPU registers and CYR/SR/CYL/EDOP editing effects; these are
fixed source data operations, not instruction fetches or an AGC decoder.

Inputs are actual bank41 with all EB banks, SUPER100/120/140/160 and IRQ0/1,
VERBREG0..99 or -0, and ordinary15-bit erasable words. Packed RUTMX routine
numbers0..13 are required only on a selected component-exchange path. Unused
routine fields can contain arbitrary words when the actual source bypasses
them. IDAD words may contain any ordinary word: LOW11 selects canonical
source erasable memory, and original register-corrected INCR handles signed
zero/overflow address edges. The canonical image is sole authority, caller
owns restoring producer exclusion, and unsupported input preserves image,
metadata and route.

The independent reference restores unchanged Pinball, noun tables, T4,
Interpreter, Executive, Display Interface and Fresh Start sources, recording
CM indentation repair and requiring whole rope equality with pinned fixtures.
8192 complete2051-field original/native/ASan/UBSan states cover source branch
selection, all routine positions, all EB/SUPER, signed NNADTEM/verb cases,
ordinary address flags and source CPU/editing aliases. The source corpus tests
all CPU-register addresses across the two missions, and frozen target cases
include every CM CPU/editing alias and the mission-specific routine13 minor
word. Exact full-stream SHA is retained in differential/full-sanitizer logs
and the frozen vector header.

Sixty-four complete original checkpoints run in strict host, sanitizer and
native C64 diagnostics. `tools/dsky_mixed_noun.mk` supplies portable host
all/test/sanitize aliases and an explicit C64 target. Stock-ROM VICE checks
actual mailbox, code/data RAM bound, zero-page allocation and hardware/software
stack sentinels. Remaining verb dispatch/application bodies are actual pending
continuations; this milestone does not claim whole Pinball completion.
