# Translate Comanche UPDATE_PROGRAM as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-update`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and an operand table only, no ABI change).

UPDATE_PROGRAM is P27, the ground uplink. The extended verbs V70 (liftoff time), V71 (contiguous block), V72 (scattered words) and V73 (AGC clock) collect the component count and data through flashing V21 N01/N02 displays, check them, and then store them under restart protection. V70 and V73 adjust TIME2/TIME1 through TIMEDIDL/TIMEDIDR. All of it is native here, one `agc_image_ops` call per original instruction. The Comanche file is Luminary's with two differences: bank 43 adds CKMDMORE (a nonzero MODREG is accepted only with FLAGWRD5 bit 8 set and no MODREG bit but bit 2), and the bank 27 body is Luminary's bank 04 body six words lower, using `CAF BIT3` for VERIFBIT and unswitched erasable (0300-0332) for the update words. `src/cm_update.c` follows `src/lm_update.c` line for line.

## Scope

- `Comanche055/UPDATE_PROGRAM.agc`, every instruction:
  - Bank 43 (3726-3765): V70UPDAT, V71UPDAT, V72UPDAT, V73UPDAT, CKMDMORE and UPERROR.
  - Bank 27 (3373-3762): UPPART2, OHWELL1, OHWELL2, UPVERIFY, CK4V32, UPSTORE, UPEND73, UPFNDVAC, UPJOB, UPWAKE, TIMEDIDL, TIMEDIDR, DELTAOK, UPEND71/STORLP71, UPEND72/LDLOOP72, UPOUT, UPEND70, UPERROUT and UPACTOFF.
- Entries (`cm_update_routines`, 35):
  - The verbs (V70-V73).
  - The POSTJUMP targets UPPART2 and UPERROUT+2.
  - The job UPJOB, and the basic code after its interpretive EXIT (UPWAKE+1).
  - The restart points UPOUT+1 (PHASCHNG's 2CADR) and TIMEDIDR (group 6).
  - The return point after every TC to code outside the file: TESTXACT, PHASCHNG, NEWMODEX, FALTON, FINDVAC, UPFLAG, TPAGREE, INTWAKEU and NEWMODEX+3, plus BANKCALL GOXDSPF with its three returns (V34, V33, data).
- Each routine runs until Z reaches one of those external entries, or ENDOFJOB, ENDEXT or INTPRET.
- Internal transfers with a run-time target go through `jump`, which accepts only the targets the original can reach:
  - `INDEX UPVERB; TCF +1` (both tables).
  - CK4V32's `TC Q` and `INDEX Q; TC 0 -6`.
  - CKMDMORE's `TC Q` (back to 3743).
  - UPACTOFF's `TC Q`.
  - TIMEDIDR's `TC UPTEMP` (the word TIMEDIDL's QXCH stored: 3534 or 3721) and `INDEX UPTEMP; TC 1`.
- `TC Q` executes the word in Q as TC and leaves Q = 3.
- Indexed stores: STORLP71's and LDLOOP72's `INDEX A; CA UPBUFF +n` and `INDEX A; LXCH 1400` form the instruction as the hardware does. It must keep its order and name an erasable word. An update that would write a register or counter (physical address below 61) refuses.
- Data: the fixed constants the file reads (CA/CAF/CS/AD/MASK, DCA NEG0) are compiled from the source listing into `src/cm_update_operands.inc` (`tools/file_operands_generate.py cm-update`) and checked against the rope. The CADRs after its BANKCALL GOXDSPF and POSTJUMP UPPART2/UPERROUT+2 are already in Lane A's `cm_interbank_site_table`, which the program uses as its operand table; the test checks all five.
- Devices:
  - UPSTORE's `RXOR LCHAN` is A xor L, as in yaAGC.
  - UPACTOFF's `WAND DSALMOUT` acts on `agc_hardware.channel11`; without `program->hardware` it refuses.
  - INHINT acts on `cm_native_io.interrupt_enabled`.

Paths with no producer: a TS that overflows refuses (no input in the program's domain overflows). CCS branches after a MASK of one bit are taken faithfully, but STORLP71's negative MPAC branch would execute `ADRES UPBUFF -1` as TC 0303 and refuses. V73 never reaches the UPVERB table after UPFLAG; UPVERB 3 there falls into TIMEDIDL as the original does.

## Source map

| AGC | Native (`src/cm_update.c`) |
|---|---|
| V70UPDAT-V73UPDAT, CKMDMORE, UPERROR (bank 43) | `bank43` |
| UPPART2 ... UPACTOFF (bank 27) | `bank27`, labels `L<address>`; `jump` for run-time targets |
| constants | `src/cm_update_operands.inc` |
| BANKCALL/POSTJUMP CADRs | `cm_interbank_site_table` (Lane A) |

## Evidence

Hashes: `evidence/cm-update/SHA256.json`. Bulk files are assets of the `evidence-cm-update` release (`python3 tools/evidence.py fetch cm-update`).

- Independent assembly comparison: `evidence/cm-update/cm-update-proof.json`. `make cm-update-differential` runs the unmodified Comanche055 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters entry n % 35, with the image pseudo-random within the program's domain:
    - UPVERB is 0-3.
    - COMPNUMB is what OHWELL1 accepts for that verb (2 for V70/V73, 3-20 for V71, odd 3-19 for V72), and UPCOUNT is 1 to COMPNUMB.
    - UPTEMP is a small component number, or TIMEDIDL's return address where TIMEDIDR uses it.
    - UPBUFF holds update addresses in erasable banks 1-7 where V71/V72 store through them.
    - MODREG is +0, -0, 2 or a mode, and FLAGWRD5 bit 8 is set in half the cases, so CKMDMORE takes all three outcomes.
    - FB/EB, SUPERBNK, TIME1/TIME2, channel 11 and the interrupt-enable state are pseudo-random.
  - Each case runs to the first point neither this file nor INTER-BANK_COMMUNICATION owns.
  - Compared: the interrupt-enable state, SUPERBNK, channel 11 and all 2,048 erasable words. The digests are frozen into `tests/cm_update_vectors.h`.
- Host and sanitizer results: `make test-cm-update sanitize-cm-update` (`evidence/cm-update/cm-update-host.log`). Beyond the oracle digests, the test checks that:
  - all 35 entries are reached;
  - each case ends at one of the 13 external entries, and every one of them is reached;
  - a TC UPTEMP or `INDEX UPTEMP; TC 1` word TIMEDIDL never stores, an UPVERB with no branch, an update into a register or counter, a missing io and UPACTOFF without the hardware record are refused.
- C64 execution results: `make vice-cm-update` (`evidence/cm-update/cm-update-vice.log`): every 16th case (256).
