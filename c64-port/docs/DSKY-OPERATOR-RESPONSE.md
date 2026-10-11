# Wider original operator response qualification

`dsky_operator_response` and `cm_operator_response` execute VBPROC,
VBPROC+1, VBTERM, VBRESEQ, and PROCKEY through KILMONON, RELDSP,
FLASHOFF, RECALTST, and genuine ENDOFJOB. Actual source entries are LM
bank 40 PCs 3442/3443/3450/3457/3452 and CM bank 40 PCs
3347/3350/3355/3364/3357. The canonical image remains authoritative;
producer exclusion belongs to the caller and no additional poll is added.

The released `dsky_load_control` already qualified the four top-level
operator controls and RECALTST. Its bytes and historical proof remain
unchanged. This separate packet adds direct VBPROC+1 with raw A, the TS
LOADSTAT overflow branch that skips TC KILMONON, explicit channels 12/13
and restart flip-flop preservation, and sequential wake preconditions.
It composes the new separately proved direct recall service.

Proceed stores LOADSTAT plus one, Terminate minus one, and resequence
minus zero. PROCKEY clears REQRET and blocks number entry with the actual
DSPCOUNT minus 19. A normal call overwrites MONSAVE1 with the kill-monitor
bit. Direct VBPROC+1 overflow skips that overwrite; RELDSP still clears
its external-monitor bit. The source channel 11 outputs release the
key-release lamp and disable flashing, with their real restart flip-flop
effects. The recalled job receives original saved-location and verb/noun
stores, including the unmatched-wake minus-one index behavior.

All EBANKs, selecting SUPER 100/120/140/160, IRQ 0/1, raw registers and
ordinary CADRSTOR/list classes are modeled. A selected wake requires a
positive active core 0, nonzero remaining priorities, canonical NEWJOB,
and legal first matching dormant bank. The CADRSTOR wake excludes the
slot already awakened by DSPLIST; an invalid bank in that now-positive
slot is not inspected again. When both request fields are empty,
unrelated Executive words remain opaque, including plus-zero priorities
and noncanonical NEWJOB. Rejected inputs preserve image, IO, event mask,
and restart state.

8,192 independent original checkpoints compare all 2,048 words, SUPER,
IRQ, channels 11/12/13, channel events, restart flip-flop, and the fixed
END route (2,056 fields). Native and full ASan/UBSan streams agree byte
for byte, SHA-256:

`4c4e62592ad19c84c0615e74293118ced958d237f8d6bdfcba42d1c75c6eb31d`.

Frozen 64 states and 16 atomic guards pass host/sanitizer. Stock-ROM PAL
C64 runs compare 32 full states per mission: LM `a500002000`, PRG
17,582 bytes (17,580 loaded), stack extents 18/747; CM `a500002001`, PRG
17,731 bytes (17,729 loaded), stack extents 19/728. Both fit below C000
without additional zero-page storage. These linked diagnostics are not
production 16 KB disk payloads. Native disk splitting and canonical
END/idle adoption remain application composition work. Fresh unchanged
source fixtures assemble to byte-identical pinned ropes; the oracle
links no tested native control bodies.

`tools/dsky_operator_response.mk` provides isolated host, test, sanitizer,
differential, and C64 targets with host-only `all`. The unchanged older
load-control APIs continue to serve their published domain.
