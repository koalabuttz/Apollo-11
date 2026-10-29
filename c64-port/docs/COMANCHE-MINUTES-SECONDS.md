# Original Comanche minutes/seconds output

`cm_minutes_seconds_phase` translates actual M/SOUT42,3216 through its
source POSTJUMP into DSPDCEND+2 at41,02570. The six compiled groups perform
fresh READLO, TPAGREE, source magnitude limiting and half-second rounding,
SEPSEC, seconds scaling, signed two-character output, the middle blank,
SEPMIN, minutes scaling, and the second signed two-character output.
The clamp includes exactly59m58.5s and forces the original59m59.5s pair
before unrounded separation. Ordinary lower values use the original signed
half-second adjustment. This preserves mixed-word signs and signed zeros.

The new reusable `cm_decimal_pair` translates actual DSPDC2NR40,3173,
including DSPSIGN, SHORTMP, two original TRACE1/DSPIN iterations and actual
caller return. Existing five-character/unsigned formatters are unchanged.
Original CM return constants and all source register, relay and scratch
stores are retained. The M/S groups reuse the qualified CM BANKCALL,
SWRETURN, POSTJUMP and generic DSPIN bodies. Passive fixed data remains
immutable; native modules contain integer arithmetic and explicit caller
routes, with no instruction decoder, heap or floating-point dependency.

The sole canonical image owns all RAM. SUPER and IRQ are explicit metadata.
Producers must be excluded; every linked frame returns before an overlay
replacement. M/S accepts normal or mixed fresh ordinary physical DP pairs
1000..3776, not crossing an erasable bank; source DECOUNT0..2 and row starts
14/9/4. Initial bank42/SUPER100/any retained EB is checked. All source MODE
words are retained. Intermediate groups require their actual source PCs,
banks and row count. Unsupported downstream callee contexts are a native
fault boundary; no general transaction rollback is promised for a composite
call after it enters a callee.

Phase ends are PC3256,3262,3264,3274,3304 in bank42 and2570 in bank41.
The source saves whole minutes/hours in HITEMOUT1007/LOTEMOUT1010, reuses
SEPSCRET/SEPMNRET0144, and writes the Pinball TPSL overflow aliasMPAC+6,
not interpreter OVFIND. Source caller bank/return debris and BUF2 are
preserved through the actual interbank helpers.

All1,024 independent signed two-character caller states and512 continuous
six-group M/S paths (3,072 states) match the original pinned rope in every
one of2,048 words plus SUPER/IRQ. Both complete streams match ASAN/UBSAN.
The raw source SHA256 values are respectively
`83332c4c578a79af68653ad4b16395e3253406046d482e45fd177a0cf232bae7`
and`ba081ba5b6885ae0578a8f178e2c505c15e8cc23cd547fbc206f5479992250d4`.
Cases cover all three display rows, eight retained EB contexts, mixed and
normal fresh reads, independently signed pairs, both zeros, rounding and
exact clamp boundaries, maximum magnitudes and random source backgrounds.
The frozen checker covers112 complete original transitions and63 unchanged
invalid contexts. Actual PAL C64 passes all112 transitions, mailbox
`a500007000`, PRG28477 bytes, measured hardware stack15 plus14 ROM bytes,
software stack177 and zero additional zero-page bytes.

Use `tools/cm_decimal_pair_differential.py`,
`tools/cm_minutes_seconds_differential.py`,
`tools/freeze_cm_minutes_seconds.py`, the host/sanitize/C64 recipes in
`tools/cm_minutes_seconds.mk`, and `tools/cm_minutes_seconds_vice_test.py`.
The original compressed streams and exact source/dependency/target hashes
are retained. This packet qualifies these bodies, not all Pinball entries
or the surrounding V06N40 consumer admission/completion path.
