# Translate Luminary DOWN-TELEMETRY_PROGRAM as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-downtelemetry`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines only, no ABI change). It follows `docs/CM-DOWNTELEMETRY.md`: Luminary099's program is the Comanche one 013 words lower, with a different DODOWNTM front end and six downlists instead of five.

The DOWNRUPT lead-in saves A and L, loads BB for bank 05 and jumps to DODOWNTM. DODOWNTM goes through WOTEST, which sets the word-order bit (channel 13 bit 7) only when it is clear, then executes `TC DNTMGOTO`: the word stored in DNTMGOTO runs as an instruction. Each phase then sends two words through channels 34 and 35 and leaves by `TCF RESUME`. All of it is native here, one `agc_image_ops` call per original instruction.

## Scope

- `Luminary099/DOWN_TELEMETRY_PROGRAM.agc`, every instruction (05,3506-3771): DODOWNTM, WO1, DNPHASE1, DNPHASE2, NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP, SNAPAGN, FETCH2WD, DOSUBLST, NEXTINSL, DNTMEXIT, DNDUMPI, DNDUMP, DNDUMP1, DNDUMP2, SENDID and WOTEST.
- `TC DNTMGOTO` is an erasable continuation (`AGC_TRANSFER_ERASABLE_BASE + 0335`). It accepts exactly the six words stored there: LDNPHAS1 (STARTSUB), LDNPHAS2, LDNDUMPI (DNEDUMP), LDNDUMP, LDNDUMP1, and the return address of DNDUMPI's `TC SENDID` (`QXCH DNTMGOTO`). Any other word refuses.
- Data: the six downlists of `DOWNLINK_LISTS.agc` (DNTABLE 05,2441-2446, the control lists, DNPTR sublists and snapshot sublists) and the constants the program reads are compiled from the listing into `src/lm_downtelemetry_data.inc` (`tools/downtelemetry_generate.py --mission Luminary099`) and checked against the rope. MINTIME2 and DNADRDCR are executed as MASK 7753 and MASK 4001 on the +0 branch of the CCS before them; the words those read are compiled too. MINB1314 would execute as CS 7777, but the source marks it "CAN'T GET HERE" and Luminary assembles no word at 07777, so that path refuses.
- Run-time INDEX: the list pointers (`INDEX CTLIST; CA 0`, `INDEX SUBLIST; 0 0`) and DNCHAN's `INDEX DNECADR; INDEX 0 -4000` form the instruction word as the hardware does (overflow-corrected sum of the sign-extended words). The result must be a CA of a compiled fixed word, or a READ of channel 11-14 or 30-33. Anything else refuses.
- Channels: `lm_dntm_io` is the mission's `p66_bias_io` control record plus channels 11-14 with the channel-13 traps, input channels 30-33 and output channels 34/35. WOR, WAND, RAND and WRITE act on it as yaAGC's CpuWriteIO does, so writing channel 13 latches the trap bits and masks them out.
- External continuation: RESUME (WAITLIST, 02,3270).

Paths with no producer: WOTEST's CCS of a masked single bit is never negative or -0 (the -0 exit refuses); the +0 branch after `AD MINTIME2` cannot occur; and the +0 branch of SNAPAGN's CCS would run LDNPHAS2 as `TC DNPHASE2`. No snapshot list ends in +0, so that path refuses.

## Source map

| AGC | Native (`src/lm_downtelemetry.c`) |
|---|---|
| DODOWNTM 05,3506, WO1 3512, WOTEST 3763 | `dodowntm` (to the erasable point DNTMGOTO) |
| `TC DNTMGOTO` (erasable 0335) | `dntmgoto` |
| DNPHASE1 3515, DNPHASE2 3523, DNDUMPI 3707, DNDUMPI+3 3712, DNDUMP 3722, DNDUMP1 3735 | `body` from the matching label, to TCF RESUME |
| NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP/SNAPAGN, FETCH2WD, DOSUBLST/NEXTINSL, DNTMEXIT, DNDUMP2, SENDID | labels `L<address>` in `body` |
| DOWNLINK_LISTS words, constants | `lm_dntm_data` (`src/lm_downtelemetry_data.inc`) |
| The constants the program's own instructions name (`k(K_...)`) | `LM_DNTM_CONSTANT` (same `.inc`, same words): `k()` folds a constant address to its word at compile time; any other address still searches `lm_dntm_data` |

## Evidence

Hashes: `evidence/lm-downtelemetry/SHA256.json`. Bulk files are assets of the `evidence-lm-downtelemetry` release (`python3 tools/evidence.py fetch lm-downtelemetry`).

- Independent assembly comparison: `evidence/lm-downtelemetry/lm-downtelemetry-proof.json`. `make lm-downtelemetry-differential` runs the unmodified Luminary099 rope (assembled from this repository by `tools/setup_cloud.sh`) in yaAGC (host-only). There are 48 sequences. 45 start at DNPHASE1 with each downlist code 0-5 and take 240 downrupts, so each downlist is sent past its end and restarted. Three start at DNDUMPI with DUMPCNT asking for 1, 2 and 4 complete dumps (1,100, 2,200 and 4,400 downrupts), which also read the bank-0 registers, including Z, as dump data. Before each downrupt, the lead-in registers (an interrupted BB in A, Q, FB 05/EB 0) and channels 11-14 and 30-33 are pseudo-random, so WOTEST sees the word-order bit both set and clear. Channel 14 leaves out the IMU CDU, optics and gyro drive bits, which yaAGC acts on as a peripheral. After every downrupt, channels 13, 34 and 35 and Q are compared (Q catches the RETURN from WOZERO, which leaves Q = 3); at the end, the traps and all 2,048 erasable words. The digests are frozen into `tests/lm_downtelemetry_vectors.h`. `evidence/lm-downtelemetry/lm-downtelemetry-data.json` lists every compiled word with its source line.
- Host and sanitizer results: `make test-lm-downtelemetry sanitize-lm-downtelemetry` (`evidence/lm-downtelemetry/lm-downtelemetry-host.log`). Beyond the oracle digests, the test checks that:
  - all eight entries are reached;
  - every control word of all six downlists is sent;
  - every dump case returns to DNPHASE2;
  - an unknown DNTMGOTO word, a DNLSTCOD with no downlist (6), a missing io and a bank other than 05 are refused.
- C64 execution results: `make vice-lm-downtelemetry` (`evidence/lm-downtelemetry/lm-downtelemetry-vice.log`): all 48 sequences, so every downlist and every dump case also runs on the C64.
