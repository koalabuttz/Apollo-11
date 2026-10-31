#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write the listing's names the readable C uses, for each mission, from the
assembled listings: src/agc/lm_names.inc (Luminary099) and
src/agc/cm_names.inc (Comanche055).

Each K_<name> is a label's logical address (a fixed label as its 12-bit Z
value, an erasable one as its 10-bit address under EB) and each KW_<name>
the fixed word at a constant's label: the same macros, read the same way,
as c64-port/tools/resident_kernel_labels.py writes for the faithful kernel,
with the names of every subsystem readable/ has added since. A name the
listing defines by EQUALS or = (TEMPG, ONE, PRIO30) is read from its symbol
table. NAME+N names the word N after a label.

  listing_names.py          write both files
  listing_names.py --check  fail if either differs from the listings
"""
import re
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import interp_listing as IL  # noqa: E402
import resident_kernel_labels as labels  # noqa: E402

# Beyond the Executive and WAITLIST (which resident_kernel_labels lists).
CODE = labels.CODE + '''
BANKCALL SWCALL SWRETURN POSTJUMP BANKJUMP Q+10000 PRIO12 MAKECADR SUPDACAL IBNKCALL ISWCALL
ISWRETRN USPRCADR SUPERSW
NEWMODEX NEWMODEA MMDSPLAY PREBJUMP CHECKMM SETUPDSP 2PHSCHNG PHASCHNG PHSCHNGA PHASJUMP
ONEORTWO OLDPRIO CON1 TOCON2 GETPRIO GETNEWNM PHSCHNG2 BELOW1 BELOW2 BELOW3 BELOW4 BIT14COM
CON2 CHECKB Q+1 Q+2 ADRPCHN2
RESTARTS GETPART2 ITSAVAR RTRNCADR ITSAWAIT TIMETEST ITSINDIR FINDTIME IMEDIATE ITSLIKEB ITSAJOB
CHKNOVAC ITSNOVAC ITSATBL CONTBL2 ITSLGCL1 ITSLGCL2 LONGCLCL ITSLNGCL ITSWTLST ITSAJOB2 ITSEVEN
PHSPART2 SIZETAB PRDTTAB CADRTAB NEWPHASE NUFAZ+10
ABS ABVAL ACOS3 ACOSABRT ACOSSHR ACOSST2 ACOS=0 ALARM ALARM1 ARCCOS AXC AXT BPL BPL/BMN BUFZERO
BVSU BZE/GOTO CALL/ITA COSINE DANZIG DIRADRES DMP1 DMPR DODLOAD DODLOAD* DOT DOTSUB DOVLOAD
DOVLOAD* DSQ DVNORM DVOVF DVXSC ENDDOT EXIT 15BITADR GENSCR GENSHIFT GOTO INCR INDJUMP LARGE2 LXA
LXC MAXTEST NOIBNKSW POLY2 RIGHTR ROUND RTB SETOVF /NORM SN1 SQRT SQRTABRT STORJUMP SUBTR SXA TEST
TIX TPAGREE TSLC2 UNIT VAD VECAGREE VPROJ VRIGHT2 VSQ VSQSUB VSSL VSSR VVXSC VXM/MXV VXV V/SC2 XAD
XCHX XSU B5TOBB BIASHI BIASLO BIT14 BIT15 LBUF2 LOW7 LVBUF NEG1/2 NO.WDS OCT176 SLOPEHI SRDDV
REVCNT LIMITS
INTPRET BHIZ DDV/BDDV INTRSM SLOAD2 NEWMODE TLOAD TMODE VMODE BRANCH OVERFLOW DMP DMPSUB POWRSERS
POLY DPAGREE SHORTMP SHORTMP2 DMPNSUB MPACVBUF ALSIGNAG DCOMP SQRTSUB SINE
ALARM2 BORTENT PRIOENT LARMENT CHKFAIL1 CHKFAIL2 FAIL3 PROGLARM MULTEXIT MULTFAIL PRIOLARM -2SEC
WHIMPER ABORT2 OCT40400 OCT77770 GOPOODOO STRTIDLE OCT1103 CURTAINS OCT217 BOTHABRT ALMNCADR
VARALARM OCT14 FLAGS PRIODSPR DOWNFLAG
SPCOS SPSIN SPT POLLEY ARG90'''.split()
ERASABLE = labels.ERASABLE + '''
BUF2 RUPTREG3 RUPTREG4 MPTEMP
MODREG RUPTREG1 RUPTREG2 TEMPG TEMPP TEMPNM TEMPBB TEMPSW TEMPSW2 TEMPPR TEMPG2 TEMPP2 TEMPBBCN
PHASE1 -PHASE1 TBASE1 PHSPRDT1 PHSNAME1 LONGBASE TIME1 TIME2
TEMPPHS TEMP2G POINTER TEMPSWCH GOLOC CYR SR
ADDRWD POLISH MODE VBUF X1 X2 S1 QPRET LV LVSQUARE DVSIGN INDEXLOC MAXDVSW DVNORMCT POLYRET POLYCNT
MATINC ESCAPE ESCAPE2 DOTRET DOTINC SWBIT SWWORD STATE TEM1 DMPNTEMP CYL
ALMCADR ITEMP1 FAILREG DSPTAB FLAGWRD3 FLAGWRD7 FLGWRD10 -PHASE4 BRUPT
TEMK SQ'''.split()
# Erasable names one mission's listing has and the other's not.
ERASABLE_ONE = 'LOCALARM BANKALRM'.split()
WORDS = labels.WORDS + '''
LOW10 BIT14COM
PRIO30 OCT7 OCT17770 BIT7 BIT12 BIT15 OCT14000 OCT60000 CON2ADR MMDSPLAY+3
PHS2CADR PRT2CADR LGCLCADR FVACCADR WTLTCADR NOVACADR OCT1400 PRIO14 OCT37776 BIT10 THREE
MINUS2 OCT177 RTRNCADR GETPART2+5 GETPART2+6
B12T14 B3TOB4 B5TOBB BANKMASK BIASHI BIASLO BIT1 BIT11 BIT13 BIT5 BIT6 CALLCODE DEC45 DLOADCOD
DLOAD* FIVE 5B10 FOUR FOURTEEN HALF HIGH4 HIGH9 LASINEX LBUF2 LDANZIG LOW11 LOW2 LOW3 LOW4 LOW7+2K
LOW8 -1/2+2 -ENDERAS -ENDVAC -OCT10 NEG12 NEG4 NEGONE OCT176 OCT23 QUARTER SEVEN SIX SLOPEHI
SLOPELO SRDDV TCSUBTR TEN THIRTEEN VLOADCOD VLOAD* ITR15+1 ITR11+1 VAD+4 VAD+11 VAD+18 LVBUF
OCT40400 V05N09 OCT35 V37FLBIT ADR40400 ADR77770 BBSERVDL STATEBIT REINTBIT NODOBIT OCT1103
C5/2 C3/2 C1/2'''.split()
WORDS2 = labels.WORDS2 + 'ADRPCHN2 600SECS'.split()
# Fixed words a routine reads through the fixed-memory hook, by physical
# address (P_<name>): bank * 02000 + offset.
PHYSICAL = 'SIZETAB 600SECS PRIOLARM FLAGS'.split()


def unique(names):
    """The names once each, in their first order."""
    return list(dict.fromkeys(names))


CODE, ERASABLE, WORDS = unique(CODE), unique(ERASABLE), unique(WORDS)


def macro(name):
    """The C spelling of a listing name: as resident_kernel_labels spells it,
    with '*' as _STAR and '=' as _EQ_ (DLOAD*, ACOS=0)."""
    return labels.macro(name.replace('*', '_STAR').replace('=', '_EQ_'))


FIXED = re.compile(r'^\d+,F:\s+(\S+)\s+(?:([0-7]{2}),)?([0-7]{4})$')


def symbols(path):
    """{name: physical fixed word} from the listing's symbol table."""
    out = {}
    text = open(path, errors='replace').read()
    for line in text[text.index('Symbol Table'):].splitlines():
        for part in line.split('\t'):
            m = FIXED.match(part.strip())
            if m:
                loc = int(m.group(3), 8)
                if m.group(2):
                    out.setdefault(m.group(1), IL.physical(int(m.group(2), 8), loc))
                elif loc >= 0o4000:
                    out.setdefault(m.group(1), IL.fixed_fixed(loc))
    return out


def generate(key, program):
    path = str(PORT / IL.LISTINGS[key])
    listing = IL.Listing(path)
    eras = labels.erasable_table(path)
    fixed = symbols(path)

    def where(name):
        base, _, offset = name.partition('+')
        if name in listing.labels:
            return listing.labels[name]
        if offset.isdigit() and base in listing.labels:
            return listing.labels[base] + int(offset)
        return fixed.get(name)

    out = ['/* SPDX-License-Identifier: MIT; generated by readable/tools/listing_names.py',
           ' * from the %s listing. Do not edit. */' % program]
    for n in CODE:
        if where(n) is not None:
            out.append('#define K_%s 0%04ou' % (macro(n), labels.logical(where(n))))
    for n in ERASABLE:
        if n not in eras:
            raise SystemExit('%s: erasable %s not in the symbol table' % (program, n))
        out.append('#define K_%s 0%04ou' % (macro(n), eras[n]))
    for n in ERASABLE_ONE:
        if n in eras:
            out.append('#define K_%s 0%04ou' % (macro(n), eras[n]))
    for n in WORDS:
        if where(n) is not None:
            out.append('#define KW_%s 0%05ou' % (macro(n), listing.word(where(n))))
    for n in WORDS2:
        if where(n) is not None:
            p = where(n)
            out.append('#define KW_%s {0%05ou,0%05ou}' % (macro(n), listing.word(p),
                                                        listing.word(p + 1)))
    for n in PHYSICAL:
        if where(n) is not None:
            out.append('#define P_%s %uu' % (macro(n), where(n)))
    return '\n'.join(out) + '\n'


def main(argv):
    bad = 0
    for key, program in labels.MISSIONS:
        text = generate(key, program)
        path = READABLE / 'src/agc' / f'{key}_names.inc'
        if '--check' in argv:
            ok = path.exists() and path.read_text() == text
            print('%s: %s %s the %s listing' % ('PASS' if ok else 'FAIL', path.relative_to(READABLE),
                                               'matches' if ok else 'differs from', program))
            bad |= not ok
        else:
            path.write_text(text)
    return bad


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
