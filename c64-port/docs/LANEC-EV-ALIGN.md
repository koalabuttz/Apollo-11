# Alignment and radar-designate verb bodies (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-align`.

## Scope

The verb bodies of `EXTENDED_VERBS.agc` that zero, coarse-align and
fine-align the IMU, drive the attitude error meters, designate and zero the
rendezvous radar and command the landing radar, with the checks they share:

- Luminary099 bank 43: VBZERO, IMUZEROK, RRZEROK, RWAITK, LRPOS2K, LRP2ALM,
  VBCOARK, OP/INERT, IMUCOARK, ICORK2, RRDESNBK, RRDESK2, RRDESEND,
  IMUFINEK, FINEK2, RDRUSECK/CHECKRR/CKRNDBIT/CHECKP22, IMUATTCK; bank 23:
  AURLOKON, NOLOKON, AURLKON1.
- Comanche055 bank 43: VBZERO, IMUZEROK, OP/INERT, VBCOARK, IMUCOARK,
  ICORK2, OPTCOARK, OPTC1, IMUFINEK, FINEK2, IMUATTCK, TRMATTCK, ATTCK1,
  CKLFTBTS, XACT0Q.

As in `lanec-ev-short`, each `case` of `lm_step`/`cm_step` in
`src/lanec_ev_align.c` is one basic block, every label and return point is
an entry, and control runs from block to block until it leaves these
bodies. `TC Q` (RDRUSECK, XACT0Q, CKLFTBTS) is a transfer to register Q.
CCS skips follow the source exactly, including LRBYBIT, which is bit 15:
its masked value is +0 or negative, and the negative case lands on
CHECKRR +1. Luminary's ATTCK2 task (bank 42, MP) is not part of this
milestone.

## Source map

| Source | Boundaries |
|---|---|
| VBZERO, VBCOARK fans; OP/INERT noun test (CM: INCR Q) | TCQ, Q+1, XACT0Q, ALM/END |
| IMUZEROK, IMUCOARK/ICORK2, IMUFINEK/FINEK2 (all GOXDSPF, EXDSPRET and IMUSTALL returns) | CKMODCAD, TESTXACT, BANKCALL, ENDEXT, GOPIN |
| RRZEROK/RWAITK, LRPOS2K/LRP2ALM, RDRUSECK chain | RDRUSECK's TC Q, BANKCALL, ALARM, ALM/END, GOPIN |
| RRDESNBK, RRDESK2 (VAC release), RRDESEND, AURLOKON | TESTXACT, XACTALM, BANKCALL, FINDVAC, CLRADMOD, DOWNFLAG, UPFLAG, MAKECADR, BLANKET, ENDOFJOB, BANKJUMP |
| LM IMUATTCK (channels 12, 31) | CHKPOOH, TESTXACT, BANKCALL, WAITLIST, ENDEXT |
| CM OPTCOARK/OPTC1 (SWSAMPLE, OPTIND), IMUATTCK/TRMATTCK/CKLFTBTS (channels 12, 30), ATTCK1 task | CKMODCAD +1, TESTXACT, FALTON, ALARM, BANKCALL, WAITLIST, IBNKCALL, TASKOVER, ENDEXT, GOPIN |

Domain: at RRDESK2 +3 the job's VAC address (PRIORITY bits 1-9) is 0 or
062-0777, so the indexed store goes to ordinary unswitched erasable.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-align/SHA256.json`.

- Independent assembly comparison: `lanec-ev-align-diff.json` (5,056
  cases: all 95 LM and 63 CM entries, 32 variants each, nouns 20, 72, 91
  and others, RADMODES, SWSAMPLE and OPTIND of each CCS class, random
  radar-use and lift-off flag bits, MODREG 22, VAC addresses, channels 12,
  30 and 31, an overflowing A into AURLOKON's first store) and
  `lanec-ev-align-oracle.crc`. gcov: every branch outcome but the
  out-of-scope guards, the `default` cases and the two LRBYBIT outcomes
  that bit 15 makes impossible.
- Host and sanitizer results: `make test-lanec-ev-align sanitize-lanec-ev-align`
  (`lanec-ev-align-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-align`, 241 frozen cases
  (every entry, both missions) on a VICE PAL C64 (`lanec-ev-align-vice.txt`).
