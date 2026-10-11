# Antenna time and CDU caller composition

`sband_flow()` compiles the original initial interpreted antenna sequence into
explicit native functions. It does not fetch, decode or dispatch interpreted
orders. The canonical image holds every source register, scratch and VAC word;
there is no private persistent numerical state.

TIME_FRONT executes LM SETPD/RTB or CM RTB/CALL's first operation and the real
SWCALL exchanges, stopping at banked LOADTIME. AFTER_TIME executes its actual
pending DANZIG continuation. LM stores TIME2/TIME1 into the TDEC1 DP pair and
enters LEMCONIC. CM retains the pending CALL and enters CDUTRIG. These interpreted
callee entries are original NEWOPS with the actual callee LOC and bank; a callee
LOC is not fabricated as a basic PC.

TIME_PIPELINE composes the two stages with the frozen native RTB_LOAD_TIME
body. CM_TIME_TRIG also executes frozen CDUTRIG: its true EXIT wrapper, live CDU
load, three cosine/sine/next groups, actual reentry and RVQ, followed by
AFTER_CDU's STCALL CSMCONIC. All internal CDU DANZIG entries retain pending
EXIT; ownership remains with the caller throughout. LEMCONIC and CSMCONIC
bodies are external orbital continuations, not no-op implementations.

Each original call saves QPRET in the selected VAC's +052 word. The actual
antenna returns are065616 for LM LEMCONIC,065660 for CM CDUTRIG and065662 for CM
CSMCONIC. STCALL's destination is FIXLOC+040, including exact old-minor debris
in L after the source exchange. SWCALL restores old BUF2 A/L; CM negative CADRs
are sign extended before the call. The source map independently checks the
five leading antenna words per mission and all compiled address bindings.

The ABI admits the five original VAC bases0401/0455/0531/0605/0661, coherent
banks, ordinary image words with raw16 A/L/Q, NEWJOB+0, and SUPER selecting
physical bank42 under0100/0120/0140/0160. Each entry checks its exact source
PC/LOC/BANKSET/pending EDOP before mutation. The caller excludes all producers
through the complete call, including native CDU internal phases. Known compiled
successor states satisfy every delegated helper precondition. Invalid entry
contexts preserve the complete image and external latches. This proves
quiesced source composition, not live Executive scheduling or arbitrary callees.

The input-only GPL host oracle uses the independently assembled original LM
and CM fixed-data layout. Neither numerical nor control native code is linked
into that oracle. Native code is MIT and contains no AGC engine or decoder.
Eight groups of4,096 cases each compare every2,048 word and SUPER/IRQ, including
4,096 joined CM time/CDU/orbital-admission calls. Full ASAN/UBSAN reproduces all
32,768 states. Contexts include all VAC bases, EB banks and selecting SUPER
forms, both IRQ values, varied signed words and raw CPU registers; there is no
exhaustive Cartesian claim.

The stock-ROM PAL C64 proof runs128 exact frozen complete states across all
eight groups. Lossless canonical source data, PRG/map, mailbox and actual target
logs are retained in the proof packet. Two test images in the standalone
diagnostic are verification storage, not a second authoritative application RAM.

Both S-band files remain partial: further coordinate transformations and angle
calculation/caller returns are still being composed, and external orbital and
display services retain their true handoff contracts.
