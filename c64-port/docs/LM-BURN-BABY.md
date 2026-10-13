# Translate Luminary BURN_BABY_BURN as transfer-ABI routines

Lane F. Milestone `lm-burn-baby`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc`, all of it,
689 assembled words:

- bank 36 `02022`-`03146`: the ignition tables P12TABLE, P40TABLE,
  P41TABLE, P42TABLE, P63TABLE and ABRTABLE, BURNBABY and B\*RNB\*B\*, the
  countdown tasks TIG-35 .. TIG-0, ULLGNOT and ULLGTASK, P40SJUNK, IGNITION
  and the program-specific ignition paths (P63IGN, P40IGN, P12IGN, ABRTIGN,
  P42IGN), DVMONCON, WAITABIT and TIGTASK, the COMFAIL family, INVFLAG,
  NOULLAGE and ONULLAGE, STCLOK1-3, CLOKTASK and CLOKJOB with the display
  branches, STOPCLOK and NULLCLOK, ASTNRETN, \*PROCEED and \*ENTER, GOPOST,
  GOCUTOFF, IGNITE, P40ALM and REP40ALM, P41SPOT's and the ABVEL display's
  interpretive code, and the file's constants;
- bank 35 `03747`-`03775`: P40AUTO, P40A/P, TURNITON and GOBACK;
- bank 17 `02000`-`02026`: TIGTASK1, P63ZOOM, P40ZOOM and P40ZOOMA;
- bank 27 `02200`-`02246`: KILLTSK2 with ADRSCAN, LETITLIV, DEAD,
  TSTFBANK and KILLDEAD;
- fixed-fixed `06027`-`06036`: KILLTASK and KILLBB.

Nothing is left in the file.

`src/lm_burn_baby.c` registers 160 basic and 14 interpretive routines
(`lm_burn_baby_routines`, `lm_burn_baby_interpretive`). One step machine
runs from each key (Z, or LOC at NEWOPS for an interpretive key) through the
file's basic code, its interpretive code and the interpreter, until control
leaves the file:

- Basic code: each instruction is one `agc_image_ops` call under a case
  label at its physical address (banks 36, 17 and 27 share the logical
  addresses), in source order. A TC or TCF within the file is a goto; one to
  another file ends the run with Q and Z set.
- A transfer to an address computed at run time sets Z and lets the step
  loop find the file's word there. That covers `INDEX WHICH; TCF n`, which
  runs the table word at WHICH + n as the original does, a `TC Q` return,
  KILLTSK2's DTCB, and NULLCLOK's `TC P40/RET`.
- RAND and WRITE of channel 11 (DSALMOUT), WOR of channel 14 and RAND of
  channel 30 act on the io record's channels as yaAGC's do. INVFLAG's
  `RXOR LCHAN` is A XOR L.
- KILLTASK's `INDEX Q; CA 0` reads the caller's CADR from
  `lm_burn_baby_sites` (the CADR after every `TC KILLTASK` in Luminary099).
- Interpretive code: each order-pair or store word is a case of its own.
  It runs the compiled `lm_interp_steps` / `lm_interp_orders` paths with the
  pair's words as literal arguments: DLOAD, DSU, STCALL, BOFF, CALL, VLOAD,
  MXV, VSR1, STOVL, VSL4, STODL, STORE and CALRB (which assembles as CALL)
  in P41SPOT, and VLOAD, ABVAL, STORE and EXIT for the ABVEL display.
  DANZIG with NEWJOB positive goes to CHANG2, as the original's does.

The keys are every label that is code and every return point of a call to
another file. A call with several returns gets a key for each one: CHECKMM's
two, GOFLASH's, CLOCPLAY's and REFLASH's three, and GOPERF1's. The
interpretive keys are LOC `36,2157`, `2163` and `2175` (the returns of
INITCDUW, CSMPREC and MUNGRAV), GOMIDAV `36,2200`, the first pair after each `TC INTPRET` (`36,2152` and
`2233`), and every word after a DANZIG, where the Executive resumes a job
that CHANG2 suspended: `36,2155`, `2166`, `2167`, `2171`, `2173`, `2177`,
`2235` and `2236`. MIDTOAV1's two returns
are basic keys at `36,2202` and `2203`.

The runs end at:

- `TC PHASCHNG` and `2PHSCHNG`;
- `TC BANKCALL`, `IBNKCALL`, `POSTJUMP` and `BANKJUMP`;
- `NOVAC`, `FINDVAC`, `WAITLIST`, `LONGCALL`, `TWIDDLE` and `FIXDELAY`;
- `TASKOVER` and `ENDOFJOB`;
- `CHECKMM`, `MAKECADR` and `TPAGREE`;
- `UPFLAG`, `DOWNFLAG`, `COMFLAG` and `DEBIT`;
- `ALARM` and `GOTOPOOH`;
- `TCF P42STAGE` and `TCF IMPLBURN` (P40-P47);
- NEWOPS outside the file: P41SPOT's CALL and STCALL targets INITCDUW,
  CSMPREC, MUNGRAV and MIDTOAV1;
- CHANG2;
- the return of a subroutine entered from outside: NOULLAGE's and
  ONULLAGE's `TC Q`, and KILLTASK's DTCB.

The operand words after the file's calls are in the shared tables:

- `TC PHASCHNG` and `2PHSCHNG` words: `src/lm_phase_sites.inc`;
- BANKCALL, IBNKCALL and POSTJUMP CADRs: `src/lm_interbank_sites.inc`;
- NOVAC, FINDVAC, WAITLIST, LONGCALL, TWIDDLE and FIXDELAY operands:
  `src/lm_exec_sites.inc`;
- the ALARM code: `src/lm_alarm_sites.inc`.

Two tables are this file's own:

- the KILLTASK callers: `src/lm_burn_baby_sites.inc`;
- the flag ADRES words after UPFLAG, DOWNFLAG and INVFLAG:
  `src/lm_burn_baby_flag_sites.inc`, `lm_burn_baby_flag_sites`.

The ignition tables are `TABLE`, and all of these are checked against the
rope.

io is `lm_bb_io` (`include/lm_burn_baby.h`). It holds the superbank bits,
the interrupt-enable bit (INHINT, RELINT, the switch orders and `TC INTPRET`
set it), channels 11, 14 and 30, and the undo image.

Refused (`EXEC_OUT_OF_SCOPE`, image and io restored from the undo image):

- a key under an FB other than its bank (KILLTASK aside);
- an interpretive key whose BANKSET or FB is not bank 36;
- a work area that is not one of the five VAC areas, PUSHLOC outside it, or
  EDOP not +0;
- NEWJOB negative at a DANZIG;
- `INDEX WHICH` with WHICH not one of the six table addresses the file's
  producers store (P12ADRES, P40ADRES, P41ADRES, P42ADRES, P63ADRES and
  ABRTABLE's WHICHADR), or a table word reached that is data, not a TCF;
- REDO4.2 with PHASE5 negative, P40SJUNK with PHASE3 negative or -0, and
  COMFAIL with PHASE6 negative. The CCS would run the next word, a CADR, as
  an instruction; PHASCHNG stores none of them.
- CLOKJOB with DISPDEX -0, or with a negative DISPDEX that does not select
  one of the display branches. The file stores -CNTDNDEX, -VB97DEX,
  -VB99DEX and -BLANKDEX, and the -25 entry is code too.
- NULLCLOK's `TC P40/RET` with P40/RET not one of the three returns its
  QXCH stores (`36,3014`, `3057`, `3074`);
- a `TC Q` (NOULLAGE, ONULLAGE) or KILLTASK with Q not a TC word into fixed
  memory, or KILLTASK with its CADR not in `lm_burn_baby_sites`;
- a computed transfer (a `TC Q` return, DTCB, `TC P40/RET`) that lands on a
  word that only runs after the EXTEND or INDEX before it. It would run a
  different instruction.
- P40AUTO's and STCLOK3's `TS` of MAKECADR's result, and KILLTSK2's
  `TS ITEMP3`, with overflow in A. MAKECADR's CADR and KILLTASK's `CA 0`
  never overflow, and with overflow the TS would skip.
- KILLTSK2's `INDEX L` (ADRSCAN, TSTFBANK, KILLDEAD) with L above 16. ZL
  and LETITLIV keep L at 0-16; a larger L (DEAD's DTCB to a word of this
  file with the caller's BBANK in L) would carry into the opcode in the
  AGC, and an odd L would never reach LSTLIM (#147). It cannot happen in
  flight, where ITEMP1 is the caller's return outside the file.
- a CCS of a MASK result that is negative (P42IGN, IGNITE, P40A/P), which
  cannot happen;
- a missing io record or undo image.

## Verification

`tests/lm_burn_baby_cases.c` builds 22,272 cases, 128 for each of the 174
keys. Each case starts from a random image with FB the key's bank (KILLTASK:
a caller's bank and Q its CADR in seven eighths of the states).

- EB is E7 (E3 at KILLTSK2), another bank in a sixteenth of the states.
  The superbank bits, the interrupt-enable bit and channels 11, 14 and 30
  vary. A overflows at the entry in an eighth of the states.
- WHICH is one of the six tables in fifteen sixteenths of the states.
  DISPDEX is +0, a count, minus one of the display indexes, or -0 in seven
  eighths. P40/RET is one of NULLCLOK's returns in fifteen sixteenths.
  PHASE3, PHASE5 and PHASE6 are each +0 or positive in seven eighths, and
  negative (-0 in a quarter of those) otherwise.
- Q and ITEMP1 are transfer destinations in switched fixed or fixed-fixed
  below the interpreter.
- The caller's BBANK at KILLTSK2 (L, or ITEMP2 past its entry) names a bank
  2-37. L counts the waitlist pairs past ADRSCAN's entry, and is any word
  in a sixteenth of the states (#147).
- In half the KILLTASK and KILLTSK2 states the waitlist holds the CADR's
  task, so KILLDEAD runs.
- At P40A/P's return from G+N,AUTO, A is +0, -0, positive or negative in
  half the states, and APSFLAG is clear in half.
- NEWJOB is positive (a job waiting) in a fifth of the states. The work
  area is one of the five VAC areas with PUSHLOC low in it, MODE one of the
  three. The drive counters yaAGC empties start +0.

`make lm-burn-baby-differential` runs `tools/lm_burn_baby_freeze.py`:

- It builds the yaAGC oracle `tools/lm_burn_baby_reference.c` on yaAGC with
  the hardware divide (`build/agc_engine_hardware_dv.o`).
- The oracle runs every case in the unmodified rope until Z leaves the
  file, KILLTASK and the interpreter (`lmbb_leaves`). The interpreter here
  is fixed-fixed `06037`-`07707` except TPAGREE, bank 0 and bank 1's
  interpretive code, and its erasable words including `TC Q` and
  `TC P40/RET` on their way. At NEWOPS, the boundary is the file's
  words.
- The native snapshot must be byte-identical for every case the routines
  accept: 21,676, with 596 refused. The comparison covers all 2,048
  erasable words, channels 11, 14 and 30 and the interrupt-enable bit.
- The digests are frozen into `tests/lm_burn_baby_vectors.h`.
- `tools/lane_f_c64_rows.py` chooses the C64 rows
  (`tests/lm_burn_baby_c64_vectors.h`, 267 cases). For each PRG it takes,
  from the cases that PRG runs as the whole file does, every key, outcome,
  path and control-flow edge of every linked source (clang
  `-fsanitize-coverage=edge,trace-pc-guard`) that those host cases take. The
  header has each PRG's rows on its own. No case reaches the other PRG's
  code. The counts are in `evidence/lm-burn-baby/lm-burn-baby-c64-cover.json`.
- The proof (`evidence/lm-burn-baby/lm-burn-baby-proof.json`) counts where
  each key's cases leave the file, and the refusals per key.

`make test-lm-burn-baby` and `make sanitize-lm-burn-baby` check every digest
(strict and ASAN/UBSAN). They require every case to leave the file or be
refused, every key, instruction, order pair and decision outcome to be
taken, and the refusals.

`make vice-lm-burn-baby` runs the C64 rows in VICE as two PRGs, because the
whole file does not fit C64 RAM:

- group 2 (`LMBB_C64_GROUP=2`) registers the bank-17 and bank-35 routines
  with their code (21 rows);
- group 1 registers bank 36, KILLTSK2 and KILLTASK (246 rows).

Both are built without inlining, with the image and the undo image in the
RAM under KERNAL ($E000, $F000; ROM banked out). The build fails if code or
BSS reaches the mailbox page. The VICE log records each PRG's SHA-256.

Paths no admitted input reaches are documented in the host test:

- the tables' fifth words (TCF TASKOVER), which no `INDEX WHICH` in
  Luminary099 selects;
- the negative and -0 outcomes of a CCS of a MASK result;
- ABVAL's alarm, which needs SQRT of a negative sum of squares.

## Source map

`tools/lm_burn_baby_source.py` reads the assembly listing and the rope and
writes `evidence/lm-burn-baby/lm-burn-baby-source-map.json`. It checks:

- every interpretive case carries the file's words as its octal literals,
  in order, from its LOC;
- every basic `AT(b,a)` or `FF(a)` label is an instruction word of the
  file. Its code holds the instruction's erasable address, branch target,
  channel, interrupt state, or the rope words of its fixed operand (both,
  for a double-word order).
- an INDEX folded into the word it indexes is read by that word's code.
- every word of the file has an owner: a label, an interpretive case, the
  word after EXTEND, a fixed operand an instruction reads, `TABLE`, a site
  operand (shared or the file's own), or a WHICH producer word
  (P40ADRES-P42ADRES) naming a table `which()` accepts;
- `src/lm_burn_baby_sites.inc` holds exactly the CADR after every
  `TC KILLTASK` in Luminary099;
- the `K_*` constants equal the rope words at their symbols, and the `X_*`
  addresses equal their symbols.

## Evidence

Hashes: `evidence/lm-burn-baby/SHA256.json`; all five files are kept in the
tree.
