# Native IMU standby and source time recovery

`imu_standby` translates the final 79 original local words per mission from
`IMU_MODE_SWITCHING_ROUTINES.agc`: the P06 flag lead-in, PRESTAND, SCALPREP,
POSTAND/POSTCOM and their ten local passive words. The additive source ledger
then accounts for all 660 Luminary and 656 Comanche words. Whole-file review
and integration remain separate from this local milestone.

The API takes the complete 2048-word canonical image plus IRQ/SUPER and
channels 11/12/13/14. Inputs require ordinary 15-bit RAM, raw 16-bit A/L,
coherent containing bank and any ordinary EB, valid IRQ/SUPER, quiescent
channel14 gyro/coarse activity, and excluded producers. Luminary bank37
requires SUPER bit0100 clear; Comanche bank26 admits the ordinary SUPER classes.
Channel13's unreadable trap-enable bits034000 must be clear. Their peripheral
latches are outside this isolated readable-I/O boundary. Unsupported inputs
remain unchanged. The API checks null objects as well as mission/stage bounds.

Twenty-one compiled source stages expose actual calls and returns. PRESTAND
inhibits interrupts, exchanges TIME2 into TIME2SAV, then calls SCALPREP. On the
ready return it exchanges MPAC into SCALSAVE, calls the original reference
flag service, clears IMUSE and RNDVZFLG through genuine DOWNFLAG frontiers,
sets channel13 bit11 and selects the actual phase and GOPERF1 bank-call entries.
Its terminate/proceed/recycle TCF paths retain Q and select the real prompt loop.
POSTAND clears channel13 bit11, clears TIME2 and invokes the second SCALPREP
caller. On readiness it subtracts SCALSAVE and selects SHORTMP followed by
TPAGREE. Negative major values receive the original BIT10 correction before
POSTCOM adds TIME2SAV. The final exchange and DAS into TIME2 precede actual
DOWNFLAG and GOTOPOOH calls. Mission-specific addresses and source TC/TCF
register effects remain explicit in the native code and complete-state proof.

SCALPREP accepts only its two genuine compiled callers. It saves Q in MPAC+2
and selects FINETIME+1. FINISH consumes the actual FINETIME return ABI: IRQ
inhibited, Q3, positive 14-bit A/L, and low scaler037777 excluded because the
original sampling loop cannot return it with producers held constant. It runs
RELINT, exchanges into MPAC, adds16, clears the minor low five bits and uses
the source CCS result to select retry or readiness. Only the two original RAM
TC returns are compiled; no instruction or opcode decoder is introduced.
The returned Q is0157, including the ready path's incremented stored return.
Helper-return stages also require their genuine Q/L/caller-frame words.

FINETIME, SHORTMP, TPAGREE, flags, phase, bank, display and GOTOPOOH bodies remain
typed external frontiers in this packet. It does not sample physical scalers,
run a standby hardware transition, execute a job or admit arbitrary callers.
Internal C returns retain source ownership and introduce no Executive check.

`imu_standby_literal` exposes ten original passive words per mission by physical
fixed address. Every 65,536 accessor argument per mission matches the original
listing/rope word map, and invalid mission/null/unowned addresses preserve output.
Original declarations, unchanged ropes and disclosed indentation-only CM repairs
remain pinned in `evidence/imu-pulse-source-audit.json`. The independent GPL
assembly engine is host-only and absent from native target binaries.

The constructor cohort contains 65,536 complete original states for each of
42 mission/stage contexts: 2,752,512 total. Each 2057-word row records identity,
IRQ/SUPER, channels11/12/14/13, status and every 2048 RAM words. Strict native
and full ASAN/UBSAN outputs match the original bytes. Varied raw registers,
channel latches, coherent banks, caller frames, time/scaler arithmetic and
nonzero unowned RAM patterns form a reproducible cohort; it does not exhaust
the Cartesian input space. One deterministic XZ retains each original stream;
native and sanitizer outputs compare directly against its decoded bytes.

Forty-two actual stock PAL C64 diagnostics check 1344 complete source-frozen
states, 504 unchanged guard calls and 420 literal values. PRGs, maps, mailbox
and measured hardware/software stack dumps are retained. Compiler ZP allocation
is zero and allocated sections end below C000. Reproduction uses isolated
`tools/imu_standby.mk` all/test/sanitize/check-host and mission/entry vice-test.
Regeneration produces separate build archives, preserving canonical evidence.
This is local source coverage; complete mission execution remains unproved.
