#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Hold every readable table of interpretive code (each struct
interpretive_code: its bank, first word and count) to the assembled
listing's extent, so that a table cannot lose its first or last word
unnoticed (issue #1304). The case tests check each word of a table
against the rope, but take the count from the table itself: a table one
word short still matches word for word.

The tables are read from the x86 objects the Makefile builds (their
values, as compiled), the listing from c64-port's assembly.lst. A table
must begin and end on whole listing lines (a 2DEC's two words together),
and the word past each end must be one the table cannot have lost:

  - a basic instruction, or a word of another listing file, or none
    (the end of the bank, or a word the listing does not assemble);
  - the first word of another table (or, before a table, the last word
    of one);
  - otherwise (a constant, or interpretive code no table holds), where
    BOUNDS below says, by the listing's label: a table one word short
    ends at a word no entry names.

  code_extents.py --lm OBJ... --cm OBJ...           check
  code_extents.py --lm OBJ... --cm OBJ... --bounds  print BOUNDS as the
                                                    tables now stand
"""
import re
import struct
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import interp_listing as IL  # noqa: E402

LISTINGS = {m: PORT / path for m, path in IL.LISTINGS.items()}

BASIC = set('''
AD ADS AUG BZF BZMF CA CAE CAF CCS COM CS DAS DCA DCOM DCS DDOUBL DIM DOUBLE DTCB DTCF DV DXCH
EDRUPT EXTEND INCR INDEX INHINT LXCH MASK MP MSK MSU NDX NOOP OVSK QXCH RAND READ RELINT RESUME
RETURN ROR RXOR SQUARE SU TC TCAA TCF TCR TS WAND WOR WRITE XCH XLQ XXALQ ZL ZQ
'''.split())

# A table's name, from the readable source: NAME or NAME[n].
TABLE = re.compile(r'const struct interpretive_code\s+(\w+)\s*(?:\[[^\]]*\])?\s*=')
# struct interpretive_code on x86_64: unsigned bank, word first, unsigned
# count, const word *words.
SIZE = 24

# The tables beside a word no table holds that is not a basic instruction
# (a constant, or interpretive code no table holds): where each begins, and
# where the word after it is, as (label, words past the label), or None
# for a side that needs no entry. --bounds prints it as the tables stand.
BOUNDS = {
    'lm': {
        'ALARMIT_CODE': (None, ('-FOURDT', 0)),
        'ALINEZ_CODE': (None, ('VB04N12', 0)),
        'APSTGO_CODE': (None, ('1SEC2D', 0)),
        'ATMAG_AFTER_EXIT_CODE': (None, ('BIT3H', 0)),
        'AVETOMID_CODE': (('AVETOMID', 0), None),
        'CAL53A_CODE': (('CAL53A', 0), None),
        'CALCRVG_CODE': (None, ('DP1/20', 0)),
        'CALCSMSC_RVQ_CODE': (None, ('XNBECADR', 0)),
        'CALSAM_CODE': (None, ('DEC227', 0)),
        'CHKSDATA_CODE': (('CHKSDATA', 0), None),
        'CHKSDA_CODE': (None, ('VB6N5', 0)),
        'COARSE_CODE': (('COARSE', 0), None),
        'COARSTYP_CODE': (None, ('OCT13', 0)),
        'CORSIT_CODE': (None, ('DISPGYRO', 0)),
        'CSI_A_CODE': (('CSI/A', 0), None),
        'DISDVLVC_EXIT_CODE': (None, ('V06N11', 0)),
        'EARTHPAD_CODE': (('EARTHPAD', 0), None),
        'ENTTIM2_CODE': (None, ('V06N16N', 0)),
        'GETULC_CODE': (('GETULC', 0), ('IMUVAR', 0)),
        'GOBAC_RVQ_CODE': (None, ('WAKESTAL', 0)),
        'GUIDINIT_CODE': (None, ('49FPS', 0)),
        'GYCOARS_CODE': (('GYCOARS', 0), None),
        'GYCOARS_DONE_CODE': (None, ('V16N20', 0)),
        'GYROTRIM_CODE': (('GYROTRIM', 0), None),
        'INTEGRV_CODE': (('CSMPREC', 0), None),
        'LOCSAM_CODE': (('LOCSAM', 0), ('CSS5', 0)),
        'LOGSUB_CODE': (('LOGSUB', 0), None),
        'LOGSUB_RVQ_CODE': (None, ('CLOG2/32', 0)),
        'LPS20_1_CODE': (('LPS20.1', 0), None),
        'LPS20_2_CODE': (('LPS20.2', 0), ('ONE/SP', 0)),
        'LSDISP_CODE': (None, ('V06N89*', 0)),
        'MANUSTAT_CODE': (('MANUSTAT', 0), None),
        'MATMOVE_CODE': (None, ('DEGREE1', 0)),
        'MEASUREMENT_INC_CODE[4]': (('DOCSM', 0), None),
        'MIDGIM_CODE': (('GET+MGA', 0), None),
        'MUNGRAV_CODE': (None, ('1.95SECS', 0)),
        'NB2CDUSP_RVQ_CODE': (None, ('16OCT', 0)),
        'NOGIMLOC_CODE': (None, ('ARATE', 0)),
        'NORMLIZ1_CODE': (('NORMLIZ1', 0), None),
        'OBLATE_CODE': (('OBLATE', 0), None),
        'ONEBASE_CODE': (None, ('V16N54', 0)),
        'OUT_CODE': (('OUT', 0), None),
        'P12INIT_CODE': (('P12INIT', 0), ('P12ADRES', 0)),
        'P21_RECYCLE_CODE': (None, ('600SEC', 0)),
        'P52LS_CODE': (('P52LS', 0), None),
        'P76SUB1_CODE': (('P76SUB1', 0), None),
        'PARAM30_CODE': (None, ('V06N33', 0)),
        'PERIAPO_CODE': (('PERIAPO1', 0), None),
        'PICAPAR_CODE': (None, ('CSS40', 0)),
        'PICEND_CODE': (('PICEND', 0), None),
        'PLANET_CODE': (('PLANET', 0), None),
        'QMINEXIT_CODE': (None, ('T/2SEC', 0)),
        'Q_GUIDSUB_CODE': (('?GUIDSUB', 0), None),
        'R51_4_CODE': (None, ('TSIGHT1', 0)),
        'R52_RETURN_CODE': (None, ('1/6TH', 0)),
        'R55RET_CODE': (None, ('V06N93', 0)),
        'R55_CODE': (('R55', 0), None),
        'RASTEER1_CODE': (None, ('2PI+3', 0)),
        'ROT180_CODE': (None, ('SINGIMLC', 0)),
        'RRDESSM_CODE': (('RRDESSM', 0), None),
        'RVBOTH_CODE': (('RVBOTH', 0), None),
        'S40_1_CODE': (None, ('EPS1', 0)),
        'S40_2_3_CODE': (('S40.2,3', 0), None),
        'SCALEVEC_CODE': (None, ('V01N14', 0)),
        'SR30_1_CODE': (None, ('MAXNM', 0)),
        'SURFSTAR_CODE': (None, ('ABOUTONE', 0)),
        'TCPIN_CODE': (('TCPIN', 0), ('OCT51', 0)),
        'THISSHIP_CODE': (None, ('MINPERM', 0)),
        'TRNSPSPD_RVQ_CODE': (None, ('MINANG', 0)),
        'UPPSV_CODE': (('UPPSV', 0), None),
        'V67WW_CODE': (None, ('V06N99', 0)),
        'WLINIT_CODE': (None, ('WBANK', 0)),
    },
    'cm': {
        'ADVTRACK_ROTA_CODE': (None, ('DP1/6', 0)),
        'AGAIN_CODE': (('AGAIN', 0), ('200CS', 0)),
        'BVECTORS_CODE': (None, ('PI/4.0', 0)),
        'CAL53A_CODE': (('CAL53A', 0), None),
        'CALCGRAV_CODE': (('CALCGRAV', 0), None),
        'CALCRVG_CODE': (None, ('KPIP', 0)),
        'CALSAM1_CODE': (None, ('DEC227', 0)),
        'CHKSDATA_CODE': (('CHKSDATA', 0), None),
        'CHKSDA_CODE': (None, ('VB6N5', 0)),
        'COARFINE_CODE': (None, ('DEGREE1', 0)),
        'CRS61_1_CODE': (('CRS61.1', 0), None),
        'CSI_A_CODE': (('CSI/A', 0), None),
        'CSI_B23_CODE': (None, ('SN359+', 0)),
        'DELRSPL_CODE': (None, ('MAXPHIC', 0)),
        'EARROT_CODE': (('EARROT1', 0), ('1/WIE', 0)),
        'EARTHPAD_CODE': (('EARTHPAD', 0), None),
        'ENTTIM2_CODE': (None, ('V06N16N', 0)),
        'FISHCALC_CODE': (('FISHCALC', 0), None),
        'GETETA_CODE': (None, ('1/12TH', 0)),
        'GETUM_CODE': (('GETUM', 0), ('DP1MIN', 0)),
        'GOTOUPSY_CODE': (('GOTOUPSY', 0), None),
        'GYCRS_DONE_CODE': (None, ('V16N20', 0)),
        'INTEGRV_CODE': (('CSMPREC', 0), None),
        'LATAZCHK_LOAD_CODE': (None, ('VNG0641', 0)),
        'LDPLANET_LOAD_CODE': (None, ('VNPLAN23', 0)),
        'LIGHTON_CODE': (None, ('RANGEB14', 0)),
        'LITTLSUB_CODE': (('LITTLSUB', 0), None),
        'LOG_RVQ_CODE': (None, ('NEARLY1', 0)),
        'MEASUREMENT_INC_CODE[4]': (('DOCSM', 0), None),
        'MIDGIM_CODE': (('MIDGIM', 0), None),
        'MIDTOAV_CODE': (('MIDTOAV2', 0), None),
        'NOPOLY_CODE': (None, ('COEFPOLY', 0)),
        'OBLATE_CODE': (('OBLATE', 0), None),
        'OUT_CODE': (('OUT', 0), None),
        'P21_RECYCLE_CODE': (None, ('600SEC', 0)),
        'P370ALRM_GOTO_CODE': (None, ('V5N09RTE', 0)),
        'P51G_CODE': (None, ('V6N22', 0)),
        'P52LS_CODE': (('P52LS', 0), None),
        'P67_2_CODE': (None, ('V16N67', 0)),
        'P76SUB1_CODE': (('P76SUB1', 0), None),
        'PARAM_CODE': (None, ('MUTABLE', 0)),
        'PERIAPO_CODE': (('PERIAPO1', 0), None),
        'PICAPAR_CODE': (None, ('SIN33', 0)),
        'PICEND_CODE': (('PICEND', 0), None),
        'PREC220_CODE': (None, ('EPC8RTE', 0)),
        'PRE_HUNT_CODE': (None, ('ADENDEXT', 0)),
        'R51B_CODE': (None, ('OCT66', 0)),
        'R53OUT_CODE': (None, ('V01N71', 0)),
        'R55RET_CODE': (None, ('V06N93', 0)),
        'R55_CODE': (('R55', 0), None),
        'R56_CODE': (('R56', 0), None),
        'R57_CODE': (('R57', 0), None),
        'R63_CODE': (None, ('PRFUNIT', 0)),
        'RECT_1_CODE': (None, ('ONE/C', 0)),
        'RENDISP4_CODE': (None, ('SXTVAR', 0)),
        'ROWDOT_CODE': (None, ('9DW26D', 0)),
        'S22LSITE_CODE': (('S22LSITE', 0), None),
        'S32_33_1_CODE': (('S32/33.1', 0), None),
        'S40_1_CODE': (None, ('THETACON', 0)),
        'S40_2_3_CODE': (None, ('TRIMSCAL', 0)),
        'S40_8_CODE': (('S40.8', 0), None),
        'S41_1_CODE': (None, ('TENBNK14', 0)),
        'S61_2_CODE': (('S61.2', 0), ('290KFT', 0)),
        'SCALEPOP_CODE': (('SCALEPOP', 0), None),
        'SHIFTNDX_CODE': (('SHIFTNDX', 0), None),
        'SR30_1_CODE': (None, ('MAXNM', 0)),
        'STARTENT_INTERPRETIVE_CODE': (None, ('500SEC', 0)),
        'STATEXTP_CODE': (('STATEXTP', 0), None),
        'STEEROFF_CODE': (('STEEROFF', 0), None),
        'STEEROFF_INTERPRETIVE_CODE': (None, ('VREFER', 0)),
        'SXTSM_CODE': (None, ('MKDNCDR', 0)),
        'TCPIN_CODE': (('TCPIN', 0), ('OCT51', 0)),
        'THISSHIP_CODE': (None, ('1/RTMUM', 0)),
        'TMRAD100_CODE': (('TMRAD100', 0), None),
        'TRNSPSPD_CODE': (None, ('MINANG', 0)),
        'V2T101_CODE': (('V2T101', 0), None),
        'VECPOINT_CODE': (('VECPOINT', 0), ('SINGIMLC', 0)),
        'WCALC_CODE': (None, ('ARATE', 0)),
    },
}


def elf_tables(path, names):
    """(name, bank, first, count) of each named table an x86_64 object
    defines."""
    data = path.read_bytes()
    if data[:5] != b'\x7fELF\x02' or data[5] != 1:
        sys.exit('code_extents: %s is not a little-endian ELF64 object' % path)
    shoff, = struct.unpack_from('<Q', data, 0x28)
    shentsize, shnum = struct.unpack_from('<HH', data, 0x3A)
    sections = [struct.unpack_from('<IIQQQQIIQQ', data, shoff + i * shentsize)
                for i in range(shnum)]
    tables = []
    for _, kind, _, _, offset, size, link, _, _, entsize in sections:
        if kind != 2:  # SHT_SYMTAB
            continue
        strings = sections[link][4]
        for j in range(size // entsize):
            name_at, info, _, index, value, length = struct.unpack_from(
                '<IBBHQQ', data, offset + j * entsize)
            if info & 0xF != 1 or not length or index >= len(sections):  # STT_OBJECT
                continue
            end = data.index(b'\0', strings + name_at)
            name = data[strings + name_at:end].decode()
            if name not in names:
                continue
            if length % SIZE:
                sys.exit('code_extents: %s in %s is %d bytes, not tables of %d'
                         % (name, path, length, SIZE, SIZE))
            base = sections[index][4] + value
            for k in range(length // SIZE):
                bank, first, count = struct.unpack_from('<IH2xI', data, base + k * SIZE)
                if bank > 0o43 or not 0o2000 <= first < 0o10000 or not count:
                    sys.exit('code_extents: %s in %s reads as %o,%o+%d, not a table'
                             % (name, path, bank, first, count))
                tables.append((name + ('[%d]' % k if length > SIZE else ''), bank, first, count))
    return tables


def physical(bank, first):
    return IL.fixed_fixed(first) if first >= 0o4000 else IL.physical(bank, first)


class Listing:
    def __init__(self, mission):
        self.listing = IL.Listing(str(LISTINGS[mission]))
        self.line_at = {}
        for line in self.listing.lines:
            for i in range(len(line.words)):
                self.line_at[line.phys + i] = (line, i)
        self.label_of = {}
        for name, phys in self.listing.labels.items():
            self.label_of.setdefault(phys, name)

    def name(self, phys):
        """The word as a label and words past it, in its file."""
        line, _ = self.line_at[phys]
        at = phys
        while at not in self.label_of:
            before = self.line_at.get(at - 1)
            if not before or before[0].src != line.src:
                return (line.src, phys - at)
            at -= 1
        return (self.label_of[at], phys - at)

    def address(self, bound):
        label, past = bound
        return self.listing.labels[label] + past if label in self.listing.labels else None

    def free(self, phys, inside):
        """True if the word at phys needs no BOUNDS entry beside the word
        inside the table: no word, another file's, or a basic one."""
        here, there = self.line_at.get(phys), self.line_at[inside]
        return not here or here[0].src != there[0].src or here[0].op.rstrip('*') in BASIC


def bounds(listing, tables):
    """{name: (begin, next)} for the tables beside a word that needs one."""
    starts = {physical(b, f) for _, b, f, _ in tables}
    ends = {physical(b, f) + c for _, b, f, c in tables}
    out = {}
    for name, bank, first, count in tables:
        begin = physical(bank, first)
        end = begin + count
        before = None if begin in ends or listing.free(begin - 1, begin) else listing.name(begin)
        after = None if end in starts or listing.free(end, end - 1) else listing.name(end)
        if before or after:
            out[name] = (before, after)
    return out


def check(mission, listing, tables):
    failures = []
    for name, bank, first, count in tables:
        begin = physical(bank, first)
        where = '%s %s %02o,%04o +%d' % (mission.upper(), name, bank, first, count)
        head, tail = listing.line_at.get(begin), listing.line_at.get(begin + count - 1)
        if not head or not tail:
            failures.append('%s: a word the listing does not assemble' % where)
            continue
        if head[1]:
            failures.append('%s: begins inside the line %r' % (where, head[0]))
        if tail[1] != len(tail[0].words) - 1:
            failures.append('%s: ends inside the line %r' % (where, tail[0]))
    want = BOUNDS[mission]
    have = bounds(listing, tables)
    for name in sorted(set(want) | set(have)):
        if want.get(name) != have.get(name):
            failures.append('%s %s: begins and ends at %s, BOUNDS says %s'
                            % (mission.upper(), name, have.get(name), want.get(name)))
        for bound in want.get(name, (None, None)):
            if bound and listing.address(bound) is None:
                failures.append('%s %s: no label %s in the listing' % (mission.upper(), name,
                                                                        bound[0]))
    return failures


def main(argv):
    objects = {'lm': [], 'cm': []}
    mission = None
    show = False
    for arg in argv[1:]:
        if arg in ('--lm', '--cm'):
            mission = arg[2:]
        elif arg == '--bounds':
            show = True
        elif mission:
            objects[mission].append(Path(arg))
        else:
            sys.exit(__doc__)
    names = set()
    for source in (READABLE / 'src').rglob('*.c'):
        names.update(TABLE.findall(source.read_text()))
    failures = []
    for mission in ('lm', 'cm'):
        tables = sorted({t for path in objects[mission] for t in elf_tables(path, names)})
        if not tables:
            sys.exit('code_extents: no %s tables (build the x86 objects first)' % mission)
        listing = Listing(mission)
        if show:
            print("    '%s': {" % mission)
            for name, pair in sorted(bounds(listing, tables).items()):
                print("        '%s': %r," % (name, pair))
            print('    },')
            continue
        found = check(mission, listing, tables)
        failures += found
        print('%s: %d tables, %d held by a label (BOUNDS)' % (mission.upper(), len(tables),
                                                              len(BOUNDS[mission])))
    if show:
        return 0
    for failure in failures:
        print('FAIL ' + failure)
    print('code-extents: %d failures' % len(failures))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
