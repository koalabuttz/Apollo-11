# Original lamp-test controls, both missions

`dsky_lamp_test` and `cm_lamp_test` translate original Verb35 control groups
from Luminary099 and Comanche055 `PINBALL_GAME_BUTTONS_AND_LIGHTS.agc`.
The CM path also translates actual `CHKPOOH` in `EXTENDED_VERBS.agc`.
These are compiled native controls on the sole 2048-word flight image.

| Original entry | Source-selected stop |
| --- | --- |
| Both VBTSTLTS41,3613 | WAITLIST LM05203 / CM05245, delay00764 (500cs), Q03651 |
| TSTLTS1 LM41,3631 / CM41,3633 | Same WAITLIST entry after original counter/relay loop |
| Both TSTLTS2 41,3662 | NOVAC LM05072 / CM05134, priority030000, Q03664 |
| Both TSTLTS3 41,3667 | Actual BANKCALL to DSPMM LM04,2636 / CM04,2566 |
| Post-DSPMM LM41,3722 / CM41,3721 | KILMONON/FLASHOFF/POSTJUMP to TSTLTS4 LM40,3471 / CM40,3376 |
| CM CHKPOOH43,2730 | Original ordinary caller for either zero MODREG; otherwise actual ALM/END through PINBRNCH10,3272 |

The on-loop fills source relay entries with072102, sets plus-sign entries
070102, and assigns NOUT11. LM and CM retain their distinct lamp masks and
IMODES addresses. Shutdown retains original channel12 NO ATT selection and
RADMODES/OPTMODES changes. KILMONON overwrites MONSAVE1 with040000. Channel11
and13 event bits are explicit; channel12 remains input. RESTART's hardware
flip-flop is cleared when an actual channel11 output includes01000; channel13
light-test state does not change that flip-flop.

All groups use any aligned EBANK and all four SUPER latches selecting the
actual source banks (100/120/140/160), canonical ordinary RAM, raw A/L/Q,
and incoming IRQ0/1. Direct TSTLTS1 accepts its original produced counter0..10.
CM CHKPOOH consumes ordinary Q0100..7777 only for either-zero MODREG; nonzero
modes ignore Q. Its TC Q leaves Q3 before SWRETURN. The lamp body translates
that literal SWRETURN exchange without changing the frozen interbank helper's
narrow ordinary-Q guard or inventing a different incoming Q. Unsupported
inputs preserve the complete image, latches, events and route.

Verification compares8192 independent unchanged-original checkpoints, every
2048 image words plus SUPER/IRQ/channel11/channel12/channel13/event-mask/
RESTART-flip-flop/selected-route (2056fields). Original fixture ropes remain
byte-identical to pinned originals; CM changes only necessary assembler
indentation. The host oracle begins with a nonzero engine cycle counter to
model an existing flight caller, avoiding the emulator's unrelated initial
power-on RESTART-light assignment. The complete native and ASan/UBSan streams
match the source byte for byte:

`8bbaf867f5a5c449e76f04e9c46e294d816a340e5ad6d583d06cbaf274429b14`.

Frozen64 source states cover both missions, entry/route, all EB/SUPER/IRQ
choices, all produced loop counters, raw register classes, zero/nonzero mode,
and hardware RESTART state. Host and sanitizer add16 atomic rejection tests.
Separate stock-ROM PAL C64 diagnostics check32 complete original states per
mission, with no extra zero-page allocation and all loaded/BSS storage below
C000. Their exact PRG/maps/mailboxes and stack-sentinel proofs are retained.

Run `make -f tools/dsky_lamp_test.mk test-dsky-lamp-test` or its `sanitize-`
variant; `differential-dsky-lamp-test` independently assembles and compares
originals then freezes source vectors. `c64-dsky-lamp-test` builds diagnostics;
`tools/dsky_lamp_test_vice_test.py --mission lm|cm` verifies actual target runs.

This packet covers the stated source groups. The WAITLIST/NOVAC consumers,
DSPMM admission/deferred mode job, TSTLTS4 release continuation, and final
native scheduling/device composition are subsequent required milestones,
not claimed completed by entry-state proofs.
