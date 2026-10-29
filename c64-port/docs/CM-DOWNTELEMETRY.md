# Translate Comanche DOWN-TELEMETRY_PROGRAM as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-downtelemetry`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines only, no ABI change).

The DOWNRUPT lead-in saves A and L, loads BB for bank 05 and jumps to DODOWNTM. DODOWNTM sets the word-order bit and executes `TC DNTMGOTO`: the word stored in DNTMGOTO runs as an instruction. Each phase then sends two words through channels 34 and 35 and leaves by `TCF RESUME`. All of it is native here, one `agc_image_ops` call per original instruction.

## Scope

- `Comanche055/DOWN-TELEMETRY_PROGRAM.agc`, every instruction (05,3521-3775): DODOWNTM, DNPHASE1, DNPHASE2, NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP, SNAPAGN, FETCH2WD, DOSUBLST, NEXTINSL, DNTMEXIT, DNDUMPI, DNDUMP, DNDUMP1, DNDUMP2 and SENDID.
- `TC DNTMGOTO` is an erasable continuation (`AGC_TRANSFER_ERASABLE_BASE + 0335`). It accepts exactly the six words stored there: LDNPHAS1 (STARTSUB), LDNPHAS2, LDNDUMPI (DNEDUMP), LDNDUMP, LDNDUMP1, and the return address of DNDUMPI's `TC SENDID` (`QXCH DNTMGOTO`). Any other word refuses.
- Data: the downlists of `DOWNLINK_LISTS.agc` (DNTABLE, the control lists, DNPTR sublists and snapshot sublists) and the constants the program reads are compiled from the listing into `src/cm_downtelemetry_data.inc` (`tools/downtelemetry_generate.py`) and checked against the rope. Three of the program's own words (MINTIME2, DNADRDCR, MINB1314) are executed as MASK 7753, MASK 4001 and CS 7777 on the +0 branch of the CCS before them. The words those read are compiled too; 07777 is the bank 03 checksum the listing reports.
- Run-time INDEX: the list pointers (`INDEX CTLIST; CA 0`, `INDEX SUBLIST; 0 0`) and DNCHAN's `INDEX DNECADR; INDEX 0 -4000` form the instruction word as the hardware does (overflow-corrected sum of the sign-extended words). The result must be a CA of a compiled fixed word, or a READ of channel 11-14 or 30-33. Anything else refuses.
- Channels: `cm_dntm_io` is the mission's `cm_native_io` (channels 11-14 and the channel-13 traps) plus channels 30-33 and 34/35. WOR, WAND and WRITE act on it as yaAGC's CpuWriteIO does, so writing channel 13 latches the trap bits and masks them out.
- External continuation: RESUME (WAITLIST).

Paths with no producer: the +0 branch after `AD MINTIME2` cannot occur, and the +0 branch of SNAPAGN's CCS would run LDNPHAS2 as `TC DNPHASE2`. No snapshot list ends in +0, so that path refuses.

## Source map

| AGC | Native (`src/cm_downtelemetry.c`) |
|---|---|
| DODOWNTM 05,3521 | `dodowntm` (to the erasable point DNTMGOTO) |
| `TC DNTMGOTO` (erasable 0335) | `dntmgoto` |
| DNPHASE1 3530, DNPHASE2 3536, DNDUMPI 3722, DNDUMPI+3 3725, DNDUMP 3735, DNDUMP1 3750 | `body` from the matching label, to TCF RESUME |
| NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP/SNAPAGN, FETCH2WD, DOSUBLST/NEXTINSL, DNTMEXIT, DNDUMP2, SENDID | labels `L<address>` in `body` |
| DOWNLINK_LISTS words, constants | `cm_dntm_data` (`src/cm_downtelemetry_data.inc`) |
| The constants the program's own instructions name (`k(K_...)`) | `CM_DNTM_CONSTANT` (same `.inc`, same words): `k()` folds a constant address to its word at compile time; any other address still searches `cm_dntm_data` |

## Evidence

Hashes: `evidence/cm-downtelemetry/SHA256.json`. Bulk files are assets of the `evidence-cm-downtelemetry` release (`python3 tools/evidence.py fetch cm-downtelemetry`).

- Independent assembly comparison: `evidence/cm-downtelemetry/cm-downtelemetry-proof.json`. `make cm-downtelemetry-differential` runs the unmodified Comanche055 rope (assembled from this repository by `tools/setup_cloud.sh`) in yaAGC (host-only). There are 48 sequences. 45 start at DNPHASE1 with each downlist code 0-4 and take 240 downrupts, so each downlist is sent past its end and restarted. Three start at DNDUMPI with DUMPCNT asking for 1, 2 and 4 complete dumps (1,100, 2,200 and 4,400 downrupts), which also read the bank-0 registers, including Z, as dump data. Before each downrupt, the lead-in registers (an interrupted BB in A, Q, FB 05/EB 0) and channels 11-14 and 30-33 are pseudo-random. Channel 14 leaves out the IMU CDU, optics and gyro drive bits, which yaAGC acts on as a peripheral. After every downrupt, channels 13, 34 and 35 and Q are compared (Q catches the RETURN from WOZERO, which leaves Q = 3); at the end, the traps and all 2,048 erasable words. The digests are frozen into `tests/cm_downtelemetry_vectors.h`. `evidence/cm-downtelemetry/cm-downtelemetry-data.json` lists every compiled word with its source line.
- Host and sanitizer results: `make test-cm-downtelemetry sanitize-cm-downtelemetry` (`evidence/cm-downtelemetry/cm-downtelemetry-host.log`). Beyond the oracle digests, the test checks that:
  - all eight entries are reached;
  - every control word of all five downlists is sent;
  - every dump case returns to DNPHASE2;
  - an unknown DNTMGOTO word, a DNLSTCOD with no downlist, a missing io and a bank other than 05 are refused.
- C64 execution results: `make vice-cm-downtelemetry` (`evidence/cm-downtelemetry/cm-downtelemetry-vice.log`): every second sequence (24 sequences, 7,720 downrupts).
