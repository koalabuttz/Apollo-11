# Translate Luminary SERVICER as transfer-ABI routines

Lane F. Milestone `lm-servicer`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/SERVICER.agc`, all of it, 1,203 assembled words:

- bank 37 `03410`-`03654`: PREREAD, BIBIBIAS, GOREADAX, READACCS, PIPSDONE,
  REDO5.5, MAKEACCS, AVEGOUT, GNUTFAZ5, GNUFAZE5 and their CADRs, then
  PIPASR, REPIP1-REPIP4, DODELVZ, REREADAC, CHKTEMX and DONEADR;
- bank 33 `02206`-`02446`: SERVICER, GETABVAL, MASSMON, MOONSPOT, AVERAGEG,
  GOSERV, COPYCYCL, DVMON with DVCNTSET, USEGTS, NODVMON1 and NODVMON2,
  LOTHRUST, DECCNTR, USEJETS, SERVOUT, XNBPIPAD, NORMLIZ1, NORMLIZ2, COPYCYC
  and TMPTOSPT;
- bank 33 `02447`-`03776`: HIGATASK, MUNRETRN, HITEST, POS2CHK, HIGATCHK,
  POS1CHK, LRPOSALM, CONTSERV, COPYCYC1, R29?, R29NODES, NOR29NOW,
  COPYCYC2, ALTCHK with its altitude table, XORCHK, 35KCHK, CALCGRAV,
  CALCGRV1, CALCRVG, RVBOTH, MUNRVG, MUNGRAV, the landing radar updates
  (UPDATCHK, POSUPDAT, NOREASON, RUPDATED, VMEASCHK, VELUPDAT, VUPDAT, USEVF,
  WSTOR, VUPDATED, VALTCHK, READV, GNURVST, QUIKFAZ5, HFAIL, VFAIL, LRVJOB,
  VBAD, VSTILBAD), HIGATJOB, POSGOOD, POSALARM and SETPOS1, SETPOS2 and
  SETPOS;
- bank 22 `03661`-`03710`: AVGEND and AVERTRN with OUTGOAVE;
- bank 27 `03321`-`03357`: SERVIDLE and SVEXTADR;
- bank 32 `03770`-`03772`: SERVEXIT;
- bank 23 `02461`-`02510`: NORMLIZE and ASCSPOT;
- bank 21 `02130`-`02153`: LRHTASK and GRP2OFF;
- bank 34 `03716`-`03774`: LRHJOB, HBAD, HSTILBAD and RDGIMS.

Nothing is left in the file.

`src/lm_servicer.c` registers 164 basic and 24 interpretive routines
(`lm_servicer_routines`, `lm_servicer_interpretive`). One step machine runs
from each key (Z, or LOC at NEWOPS for an interpretive key) through the
file's basic code, its interpretive code and the interpreter, until control
leaves the file:

- Basic code: each instruction is one `agc_image_ops` call under a case
  label at its physical address, in source order. A TC or TCF within the
  file is a goto; one to another file ends the run with Q and Z set.
- A transfer to an address computed at run time sets Z and lets the step
  loop find the file's word there: `TC Q` and `TC A` returns into the file,
  `TC LRADRET`, and AVGEXIT's DTCB.
- WOR and WAND of channel 11 (DSALMOUT), and RAND of channels 12, 32 and 33,
  act on the io record's channels as yaAGC's do.
- Interpretive code: each order-pair or store word is a case of its own. It
  runs the compiled `lm_interp_steps` / `lm_interp_orders` paths with the
  pair's words as literal arguments. That covers GETABVAL, AVERAGEG and
  AVETOMID's call, NORMLIZE, the R29 and altitude block, CALCGRAV,
  CALCGRV1, CALCRVG, RVBOTH, MUNRVG, MUNGRAV, the landing radar updates and
  SETPOS. DANZIG with NEWJOB positive goes to CHANG2, as the original's
  does. RTB as the right order of a pair (ALTCONV, ARCONV1) leaves the file
  at SGNAGREE through SWCALL.

The keys are every label that is code and every return point of a call:

- to another file, each return of a multi-return callee included (BRANCH,
  RADSTALL, PRIOLARM and BANKCALL of ALTCHK);
- within the file: TC QUIKFAZ5, GNURVST, COPYCYC, SETPOS and the rest, and
  the interpretive CALLs and STCALLs of CALCGRAV, CALCRVG, MUNGRAV and
  AVETOMID. These 25 keys exist so that each C64 segment group (below) can
  be entered at every word its own code returns to.

The runs end at:

- `TC PHASCHNG` and `2PHSCHNG`;
- `TC BANKCALL`, `IBNKCALL`, `POSTJUMP`, `BANKJUMP` and `MAKECADR`;
- `NOVAC`, `FINDVAC`, `PRIOCHNG`, `WAITLIST` and `VARDELAY`;
- `TASKOVER` and `ENDOFJOB`;
- `UPFLAG` and `DOWNFLAG`;
- `ALARM`, `WHIMPER` and `GOTOPOOH`;
- the interpreter's subroutines the basic code calls: `SHORTMP`,
  `SHORTMP2`, `ALSIGNAG`, `BRANCH` (both returns), and `SWRETURN`;
- `TCF R10,R11A` and `TCF R29`;
- NEWOPS outside the file (a CALL or STCALL to another file's code);
- SGNAGREE through RTB;
- CHANG2;
- the return of a subroutine entered from outside (`TC Q`, `TC A`,
  `TC LRADRET`, the DTCB through AVGEXIT, BANKJUMP's CADR).

The operand words after the file's calls are in the shared tables:

- `TC PHASCHNG` and `2PHSCHNG` words: `src/lm_phase_sites.inc`;
- BANKCALL, IBNKCALL and POSTJUMP CADRs: `src/lm_interbank_sites.inc`;
- NOVAC, FINDVAC, WAITLIST and VARDELAY operands: `src/lm_exec_sites.inc`;
- the ALARM codes: `src/lm_alarm_sites.inc`.

The flag ADRES words after UPFLAG and DOWNFLAG are the file's own table,
`src/lm_servicer_flag_sites.inc` (`lm_servicer_flag_sites`). All of these
are checked against the rope.

io is `lm_sv_io` (`include/lm_servicer.h`). It holds the interrupt-enable
bit (INHINT, RELINT and `TC INTPRET` set it), channels 11, 12, 32 and 33,
and the undo image.

Refused (`EXEC_OUT_OF_SCOPE`, image and io restored from the undo image):

- an interpretive key whose BANKSET or FB is not its bank;
- a work area that is not one of the five VAC areas, PUSHLOC outside it,
  or EDOP not +0;
- NEWJOB negative at a DANZIG;
- a fixed operand outside the constants the file's orders name;
- an erasable operand whose words run past the end of erasable into fixed
  memory: an indexed address such as `DMP* VZSCAL,2` with an index no
  producer stores;
- an INDEX whose value is not one the file's producers store: ITEMP1 in
  COPYCYC (an even count up to 22), A at ALTCHK (0, 2, 4 or 6), FIXLOC at
  VELUPDAT (a VAC area), VSELECT\* (0, 1 or 2), BUF in GNURVST (0 or 6), Q
  in SETPOS (0 or 2);
- a `TC Q`, `TC A` or `TC LRADRET` whose word is not a TC into fixed
  memory;
- LOTHRUST with PHASE4 negative. Its CCS would run the PHASCHNG operand
  word as an instruction.
- a computed transfer, a TS skip on overflow or a CCS outcome that lands on
  a word that runs only after the EXTEND or INDEX before it (STILBADH or
  STILBADV -0, for one). It would run a different instruction.
- a missing io record or undo image.

## Verification

`tests/lm_servicer_cases.c` builds 24,064 cases, 128 for each of the 188
keys. Each case starts from a random image with FB the key's bank.

- EB is the key's (E7, or E4 at RDGIMS and VELUPDAT's interpretive part,
  E5 at USEVF and WSTOR), another bank in a sixteenth of the states. The
  interrupt-enable bit and channels 11, 12, 32 and 33 vary.
- Q is a return into the file (after one of its calls of GNUFAZE5,
  QUIKFAZ5, COPYCYC, PIPASR, TMPTOSPT, GNURVST or SETPOS) or a transfer
  destination in switched fixed or fixed-fixed below the interpreter.
- In fifteen sixteenths of the states the indexes are what the producers
  store:
  - ITEMP1 is an even count up to 20;
  - VSELECT and VSELECT\* are 0, 1 or 2;
  - BUF is 0 or 6;
  - LRADRET is a return;
  - X1 and X2 are VELUPDAT's -6 VSELECT and -2 VSELECT at its two
    interpretive returns, and CALCGRAV's -RTX2 at CALCGRV1.
- A at ALTCHK, SETPOS and GNURVST's callers is one of the values the
  callers load, in seven eighths of the states.
- The PHASE words are +0 or positive in three quarters of the states.
- The values the file's CCS and BZF tests single out are each set in a
  quarter of the states:
  - PIPAGE, DELVZ, DELVY, TEMZ and TEMX +0 or -0;
  - DVCNTR +0;
  - STILBADH and STILBADV +0;
  - PHASE2 and PHASE4 -0;
  - TIME5 at READACCS's +0 and -0;
  - the radar counters at most 3 past their limits, or +0.
- RTX2 is 0 (earth) in a quarter of the states and 2 (moon) in half.
- REREADAC's chain of tests runs through all its outcomes by bits of the
  state number.
- NEWJOB is positive (a job waiting) in a fifth of the states. The work
  area is one of the five VAC areas with PUSHLOC low in it, MODE one of the
  three.

`make lm-servicer-differential` runs `tools/lm_servicer_freeze.py`:

- It builds the yaAGC oracle `tools/lm_servicer_reference.c` on yaAGC with
  the hardware divide (`build/agc_engine_hardware_dv.o`).
- The oracle runs every case in the unmodified rope until Z leaves the
  file and the interpreter (`lmsv_leaves`). The interpreter here is
  fixed-fixed `06037`-`07707`, bank 0 and bank 1's interpretive code, SWCALL
  after an RTB, and its erasable words including `TC Q` and `TC A` on their
  way. At NEWOPS, the boundary is the file's words.
- The native snapshot must be byte-identical for every case the routines
  accept: 23,032, with 1,032 refused. The comparison covers all 2,048
  erasable words, channels 11, 12, 32 and 33 and the interrupt-enable bit.
- The digests are frozen into `tests/lm_servicer_vectors.h`.
- The proof (`evidence/lm-servicer/lm-servicer-proof.json`) counts where
  each key's cases leave the file, the refusals per key, and the segment
  groups below.

`make test-lm-servicer` and `make sanitize-lm-servicer` check every digest
(strict and ASAN/UBSAN). They require every case to leave the file or be
refused, every key, instruction, order pair and decision outcome to be
taken, and 17 refusals, each on a case that runs to the end unedited.

Paths no admitted input reaches are listed in the host test:

- ABVAL's, ABS's and UNIT's alarm 1302, SQRTSUB's abort on a negative sum of
  squares;
- READACCS's CCS of +0, since TIME5 - 037771 is never +0 in ones'
  complement;
- the negative and -0 outcomes of a CCS of a MASK result, of ITEMP1 and of
  VSELECT, counts the file stores as 0 to 20 and 0 to 2;
- LOTHRUST's negative PHASE4 and the -0 of STILBADV and STILBADH, which are
  refused.

Two checks in the code cannot fire on any input the oracle accepts, and are
kept so a wrong input refuses instead of running a data word: the even-count
test of ITEMP1 in COPYCYC, and the TC test of A before `TC A`.

## C64 segment groups

The whole file does not fit C64 RAM. `make c64-lm-servicer` builds seven
PRGs, one per segment group (`LMSV_C64_GROUP`, `SV_G(g)` in
`src/lm_servicer.c`):

- group 1 holds the basic code outside the interpretive passages (149
  keys);
- groups 2-7 hold the interpretive passages with the basic code between
  them. Each was packed by trial builds to leave at least 1 KB free:
  - 2: CALCGRAV, POSUPDAT's NOREASON interpretive part, VELUPDAT's first
    pair;
  - 3: CALCGRV1, VUPDAT's interpretive part;
  - 4: NORMLIZ1 and NORMLIZ2, MUNRETRN, NOR29NOW, MUNRVG, SETPOS's
    interpretive part;
  - 5: AVGEND, NORMLIZE, AVERAGEG, R29NODES;
  - 6: GETABVAL, COPYCYC2, MUNGRAV, POSUPDAT's interpretive part, SETPOS2;
  - 7: ASCSPOT, CALCRVG, RVBOTH, POSUPDAT, RUPDATED, WSTOR and VUPDATED,
    SETPOS1.

A group build registers only its own keys, in the same sorted order, and
`lmsv_run` returns 4 for a case whose key is not in the build. A run that
reaches a word of another group stops there, with Z (or LOC at NEWOPS) at
that word. The `SEG_LABELS` bitmaps say which words belong to another
group; any other unknown word is refused, as in the full build.

`tools/lm_servicer_freeze.py` checks each group against the oracle on its
own terms:

- It builds each group for the host (`build/lm-servicer-native-g<n>`).
- It runs the oracle with a segment file that names the group's keys and
  every word outside it. The oracle stops when Z reaches one of those basic
  words, or NEWOPS with LOC at one of those interpretive words.
- Every case must again be byte-identical:

| Group | Keys | Cases | Equal | Refused |
|---|---|---|---|---|
| 1 | 149 | 19,072 | 18,646 | 426 |
| 2 | 3 | 384 | 377 | 7 |
| 3 | 2 | 256 | 207 | 49 |
| 4 | 7 | 896 | 854 | 42 |
| 5 | 7 | 896 | 881 | 15 |
| 6 | 5 | 640 | 586 | 54 |
| 7 | 15 | 1,920 | 1,759 | 161 |

- `tools/lane_f_c64_rows.py` chooses the C64 rows
  (`tests/lm_servicer_c64_vectors.h`, 261 rows in per-group blocks). For
  each group it takes every entry, outcome, branch and basic block that
  group's host cases take. Together they cover the 1,132 branches of the
  whole file. The counts are in
  `evidence/lm-servicer/lm-servicer-c64-cover.json`.

All seven PRGs are built without inlining and with llvm-mos's static stack,
which saves about 1.7 KB. The image and the undo image sit in the RAM under
KERNAL ($E000, $F000; ROM banked out). The build fails if code or BSS
reaches the mailbox page at $C000. `make vice-lm-servicer` runs them in
VICE. The VICE log records each PRG's SHA-256 and where its BSS ends.

## Source map

`tools/lm_servicer_source.py` reads the assembly listing and the rope and
writes `evidence/lm-servicer/lm-servicer-source-map.json`. It checks:

- Every interpretive case carries the file's words as its octal literals,
  in order, from its LOC. A word named twice in a row is a pushdown operand
  or the STORE word STADR consumes.
- Every basic `AT(b,a)` label is an instruction word of the file. Its code
  holds the instruction's erasable address, branch target, channel,
  interrupt state, or the rope word of its fixed operand.
- Every word of the file has an owner: a label, an interpretive case, the
  word after EXTEND, a fixed operand an instruction reads, a word of the
  interpretive fixed-operand table, or a site operand (shared or the file's
  own flag table).
- The `K_*` constants equal the rope words at their symbols, and the `X_*`
  addresses equal their symbols.

Two instructions are exempt from the operand-in-code check, because their
operand is not a name the code spells:

- `DXCH Z` at `33,2413`, which is the transfer itself;
- `DCA APSVEX` at `33,2243`, which reads `K_DPSVEX`. The checker matches
  that as APSVEX +1.

## Evidence

Hashes: `evidence/lm-servicer/SHA256.json`; all five files are kept in the
tree.
