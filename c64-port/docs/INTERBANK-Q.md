# INTER-BANK entries with an unusual Q (lane A)

Two INTER-BANK entries are reached with a Q outside the ordinary compiled
return range 0100..07775 that the LM and CM interbank certificates were
proved over (issue #129):

| entry | how it is reached | Q on entry |
|---|---|---|
| `SWCALL -1` (LM 04621, CM 04665) | every RTB: `TC SWCALL -1` at INTERPRETER RTB, through BANKCALL's `INCR Q` | the return address; `INCR Q` adds 1 |
| `BANKJUMP` (LM 04640, CM 04704) | DISPLAY_INTERFACE SAVELOCS by `TC L` (Q = 2), and `TC SUPERSW` then `TCF BANKJUMP` (Q = 3) | any; BANKJUMP only `XCH`es it into A as debris |

`SWRETURN` and `ISWRETRN` already accept any Q (a callee's `TC Q` reaches
them with Q = 3; #90).

## Native

- `SWCALL -1` is a registered routine in `lm_interbank_routines` and
  `cm_interbank_routines` (14 entries each, sorted by address). It performs
  `INCR Q` and continues at SWCALL, which the existing SWCALL routine takes.
  It refuses Q 07777 (the increment would leave the compiled range).
- `BANKJUMP` in `src/lm_interbank.c` and `src/cm_interbank_complete.c` takes
  Q 0..07775 and refuses 07776 and above, as before.

## Proof

`tools/interbank_q_freeze.py` runs the 1,024 cases of
`tests/interbank_q_cases.c` (256 per entry kind and mission; random erasable
image, coherent banks, A the callee's CADR in banks 04-37, both superbank
settings; half the BANKJUMP cases with Q below 0100) natively through
`agc_transfer_run` and in yaAGC from the original ropes, stopping where Z and
FB first equal the native callee. All 1,024 reach the callee and all 2048
erasable words are identical. The proof
(`evidence/interbank-q/interbank-q-proof.json`) lists the hashes of every
native source it depends on, including `src/lm_interbank.c` and
`src/cm_interbank_complete.c`.

`tests/test_interbank_q.c` checks the frozen digests and the refusals at the
Q edge (SWCALL -1 at Q 07777, BANKJUMP at Q 07776 and 07777): 1,030 checks.
The C64 diagnostic runs the first 128 cases (every kind, both BANKJUMP Q
ranges) and the refusals.
