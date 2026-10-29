# Native component blanking and bank return

`dsky_blank` and `cm_blank` execute original Luminary099 and Comanche055
`BLNKSUB1` and `SUPDXCHZ+1`, respectively. They preserve the physical canonical
2048-word image and explicit source SUPER/interrupt metadata. These native C
bodies never fetch or decode original instructions. Original assembly runs only
in the independent host reference process.

`BLNKSUB1` starts at LM bank40,3505 or CM40,3412 with SUPER100 and any retained
EB0..7. NVTEMP0..7 selects R1/R2/R3. Selected components execute real 5BLANK,
DSPIN and both 2BLANK calls, clear their source register/decimal state, update
NOUT with original signed-zero/overflow semantics, and restore DSPCOUNT.
Selected isolated DSPIN relay words must be nonzero; paired blank relay words
accept clean +0 and -0. The saved BUF pair is opaque, including both signed
zeros. The body stops at the actual LM5166/CM5230 `SUPDXCHZ+1` entry after DXCH;
it does not infer a caller or complete an Executive job.

The separate return entry executes actual WRITE SUPER, TS BBANK and TC L. It
accepts ordinary sign-extended A with only BBCON bits076167, ordinary L0100..07777,
aligned source EB/BB and legal SUPER/interrupt metadata. It restores actual bank
and SUPER bits, leaves Q2 and transfers to the real source-selected PC. No
scheduler poll or callback occurs. Invalid arguments preserve image and IO.

The original KEYREL CADRSTOR=-0 fallthrough reaches this real blanking body.
KEYREL does not initialize BUF. Thus blanking supports opaque stale BUF data,
while the separate return bridge has explicit valid bank/PC requirements.
Reachable stale return words outside this return domain remain an application
closure requirement; they are not assumed unreachable. The ordinary BLANKSUB
lead-in which captures the caller pair is a remaining separate source service.
PINBRNCH, ENTER and wider noun/verb fanout also remain separate work.

Verification uses unchanged original Pinball, T4, Interpreter, Executive,
Display Interface and Fresh Start sources for each program. CM assembly
normalization changes indentation only; each rebuilt whole rope must equal its
pinned reference fixture. All 8192 checkpoints compare all 2048 physical words,
SUPER and interrupt enable, including every component mask, all EB banks,
source raw L and saved BUF edge values, both NOUT/count zeros and overflow
boundaries, every return FB bank and SUPER domain. Full original/reference,
native and ASan/UBSan streams are identical, SHA256:
`d5642a5242d6f9b0cc8ed0ffa59f024d2d2cfc0fcd49d4390e81bd2895aaa876`.

The frozen 64 original checkpoints are compiled into strict host, sanitizer and
native C64 diagnostic tests. `tools/dsky_blank.mk` adds portable host `all`,
`test` and `sanitize` aliases; C64 is explicit. The VICE runner checks the
actual target mailbox, code/data RAM limit, zero-page budget and both stack
sentinel regions. This milestone covers source blanking and return, without
claiming a complete Pinball application.
