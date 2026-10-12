# Translate Luminary UPDATE_PROGRAM as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-update`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change).

UPDATE_PROGRAM is P27, the ground uplink. The extended verbs V70 (liftoff time), V71 (contiguous block), V72 (scattered words) and V73 (AGC clock) collect the component count and data through flashing V21 N01/N02 displays, check them, and then store them under restart protection. V70 and V73 adjust TIME2/TIME1 through TIMEDIDL/TIMEDIDR. All of it is native here, one `agc_image_ops` call per original instruction.

## Scope

- `Luminary099/UPDATE_PROGRAM.agc`, every instruction:
  - Bank 43 (3724-3751): V70UPDAT, V71UPDAT, V72UPDAT, V73UPDAT and UPERROR.
  - Bank 04 (3401-3770): UPPART2, OHWELL1, OHWELL2, UPVERIFY, CK4V32, UPSTORE, UPEND73, UPFNDVAC, UPJOB, UPWAKE, TIMEDIDL, TIMEDIDR, DELTATOK, UPEND71/STORLP71, UPEND72/LDLOOP72, UPOUT, UPEND70, UPERROUT and UPACTOFF.
- Entries (`lm_update_routines`, 35):
  - The verbs (V70-V73).
  - The POSTJUMP targets UPPART2 and UPERROUT+2.
  - The job UPJOB, and the basic code after its interpretive EXIT (UPWAKE+1).
  - The restart points UPOUT+1 (PHASCHNG's 2CADR) and TIMEDIDR (group 6).
  - The return point after every TC to code outside the file: TESTXACT, PHASCHNG, NEWMODEX, FALTON, FINDVAC, UPFLAG, TPAGREE, INTWAKEU and NEWMODEX+3, plus BANKCALL GOXDSPF with its three returns (V34, V33, data).
- Each routine runs until Z reaches one of those external entries, or ENDOFJOB, ENDEXT or INTPRET.
- Internal transfers with a run-time target go through `jump`, which accepts only the targets the original can reach:
  - `INDEX UPVERB; TCF +1` (both tables).
  - CK4V32's `TC Q` and `INDEX Q; TC 0 -6`.
  - UPACTOFF's `TC Q`.
  - TIMEDIDR's `TC UPTEMP` (the word TIMEDIDL's QXCH stored: 3542 or 3727) and `INDEX UPTEMP; TC 1`.
- `TC Q` executes the word in Q as TC and leaves Q = 3.
- Indexed stores: STORLP71's and LDLOOP72's `INDEX A; CA UPBUFF +n` and `INDEX A; LXCH 1400` form the instruction as the hardware does. It must keep its order and name an erasable word. An update that would write a register or counter (physical address below 61) refuses.
- Data: the CADRs after the file's own TC BANKCALL/POSTJUMP and the fixed constants it reads (CA/CAF/CS/AD/MASK, DCA NEG0) are compiled from the source listing into `src/lm_update_operands.inc` (`tools/file_operands_generate.py lm-update`) and checked against the rope.
- Devices:
  - UPSTORE's `RXOR LCHAN` is A xor L, as in yaAGC.
  - UPACTOFF's `WAND DSALMOUT` acts on `agc_hardware.channel11`; without `program->hardware` it refuses.
  - INHINT acts on `p66_bias_io.interrupt_enabled`.

Paths with no producer: a TS that overflows refuses (no input in the program's domain overflows). CCS branches after a MASK of one bit are taken faithfully, but STORLP71's negative MPAC branch would execute `ADRES UPBUFF -1` as TC 1173 and refuses. V73 never reaches the UPVERB table after UPFLAG; UPVERB 3 there falls into TIMEDIDL as the original does.

## Source map

| AGC | Native (`src/lm_update.c`) |
|---|---|
| V70UPDAT-V73UPDAT, UPERROR (bank 43) | `bank43` |
| UPPART2 ... UPACTOFF (bank 04) | `bank04`, labels `L<address>`; `jump` for run-time targets |
| own BANKCALL/POSTJUMP CADRs, constants | `lm_update_operands` (`src/lm_update_operands.inc`) |

## Evidence

Hashes: `evidence/lm-update/SHA256.json`. Bulk files are assets of the `evidence-lm-update` release (`python3 tools/evidence.py fetch lm-update`).

- Independent assembly comparison: `evidence/lm-update/lm-update-proof.json`. `make lm-update-differential` runs the unmodified Luminary099 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 35, with the image pseudo-random within the program's domain:
    - UPVERB is 0-3.
    - COMPNUMB is what OHWELL1 accepts for that verb (2 for V70/V73, 3-20 for V71 and V72; UPEND72 rejects an even count), and UPCOUNT is 1 to COMPNUMB.
    - After the II load (CK4V32 returning to 3432) UPBUFF is often a small number: below 3, 3-20 or above 20.
    - UPTEMP is a small component number, or TIMEDIDL's return address where TIMEDIDR uses it.
    - UPBUFF holds update addresses in any erasable bank where V71/V72 store through them. EB0 addresses start at 61, above the registers and counters, and keep clear of MPAC, which the loop itself counts in. A quarter of the V71 cases load one word at a bank's first word (UPEND71's `BZF STORLP71`, A = -0). Another quarter start within COMPNUMB words of a bank's end, so some run past it into UPEND71's EBANK-overflow error.
    - MODREG is +0, -0 or a mode.
    - FB/EB, SUPERBNK, TIME1/TIME2, channel 11 and the interrupt-enable state are pseudo-random.
  - Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns.
  - The oracle also traces the words each case executes (`tools/case_trace.h`). The freeze fails if any instruction of the file is never executed. The only words left unexecuted are constants, CADRs, interpretive words and the instruction after an EXTEND or INDEX, which runs in the same step as its prefix. The list is in the proof.
  - Compared: the interrupt-enable state, SUPERBNK, channel 11 and all 2,048 erasable words. The digests are frozen into `tests/lm_update_vectors.h`.
- Host and sanitizer results: `make test-lm-update sanitize-lm-update` (`evidence/lm-update/lm-update-host.log`). Beyond the oracle digests, the test checks that:
  - all 35 entries are reached;
  - each case ends at one of the 13 external entries, and every one of them is reached;
  - the C64 rows are host cases with their host digests;
  - a TC UPTEMP or `INDEX UPTEMP; TC 1` word TIMEDIDL never stores, an UPVERB with no branch, an update into a register or counter, a missing io and UPACTOFF without the hardware record are refused.
- C64 execution results: `make vice-lm-update` (`evidence/lm-update/lm-update-vice.log`). The rows are chosen by outcome (`tools/c64_rows.py`): every 16th case, plus greedily added cases until the rows together execute every word the 4,096 host cases execute. Each branch outcome lands on a word of its own, so the C64 takes every outcome the host takes.
