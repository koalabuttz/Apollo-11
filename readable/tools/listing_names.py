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
SPCOS SPSIN SPT POLLEY ARG90
UPENT2 DOWNENT2 UPFLAG COMFLAG DEBIT DELAYJOB OK2DELAY TCGETCAD WAKER GENTRAN B5OFF TRFAILOF
TRFAILON LOWIDCOD
DODOWNTM WO1 WOTEST DNPHASE1 DNPHASE2 DODNADR MINTIME2 DNADRDCR CHKLIST NEWLIST NEXTINCL MINB1314
SETWO DODNCHAN WOZERO DODNPTR SNAPLOOP SNAPAGN LDNPHAS2 SNAPEND FETCH2WD DOSUBLST NEXTINSL
DNTMEXIT TMEXITL TMRESUME DNDUMPI LDNDUMP LDNDUMP1 DNDUMP DNDUMP1 DNDUMP2 SENDID DNTABLE
SLAP1 STARTSW SKIPSIM DOFSTART DOFSTRT1 ENDRSTRT MR.KLEAN P00KLEAN V37KLEAN GOPROG BUTTONS DORSTART
SETINFL ELRSKIP NOCOARSE ENEMA GOPROG2 GOPROG2A GOPROG3 PCLOOP NXTRST PACTIVE PINACT PTBAD LIGHTSET
NONAVKEY STARTSUB STARTSB1 STARTSB2 DSPOFF T5IDLOC GOTOPOOH STOPRATE NOQRSM
GOP00FIX V37 V37BAD CANTROD CANTR00 CHECKTAB AGAINMM ISSERVON V37RET CANV37 R00 ROC DUMMYAD POOH
SEUDOP00 GROUPKIL GOMOD GOGOPROG RENDV00 RESET22 KILL2 KILL20 REV37 P00FIZZ RENDN00 STATQUO NOUVEAU
V37NONO V37XEQ V37XEQC SETUP70 SETUP71 INITSUB SETMAXER RAKE VAC5STOR V5LOOP1 V5OUT1 V5LOOP2 V5OUT2
R00TOP00 ISITP00 CLEARMRK RELDSP PHSCHNGA CLRADMOD FALTON SPVAC NEWMODEA DOWNFLAG UPFLAG GENTRAN
STOPRATE INTPRET ENDOFJOB ALARM PHASCHNG
KEYRUPT1 KEYCOM ACCEPTUP UPRUPT UPRPT1 UPOK CLUPLOCK TMFAIL2 UPTEST HI10 ELRCODE LODSAMPT
DOT6RUPT T6JOBCHK T6JOB ENABLET6 GOCH56 WRITEP WRITEU WRITEV T4RUPT MARKRUPT RADAREAD PITFALL
VHFREAD T5RUPT 00314OCT
NORMT4 CDRVE DSPOUTSB DSPSCAN DSPLAY DSPOUT HANG20 SETTIME4 PROCEEDE T4JUMP NODSPOUT QUIKDSP
QUIKRUPT NODSPY SYNCT4 QUIKOFF OCT37737 TABLNTH 120MRUPT RCSMONIT RRAUTCHK IMUMON DAPT4S OPTTEST
OPTMON'''.split()
ERASABLE = labels.ERASABLE + '''
BUF2 RUPTREG3 RUPTREG4 MPTEMP
MODREG RUPTREG1 RUPTREG2 TEMPG TEMPP TEMPNM TEMPBB TEMPSW TEMPSW2 TEMPPR TEMPG2 TEMPP2 TEMPBBCN
PHASE1 -PHASE1 TBASE1 PHSPRDT1 PHSNAME1 LONGBASE TIME1 TIME2
TEMPPHS TEMP2G POINTER TEMPSWCH GOLOC CYR SR
ADDRWD POLISH MODE VBUF X1 X2 S1 QPRET LV LVSQUARE DVSIGN INDEXLOC MAXDVSW DVNORMCT POLYRET POLYCNT
MATINC ESCAPE ESCAPE2 DOTRET DOTINC SWBIT SWWORD STATE TEM1 DMPNTEMP CYL
ALMCADR ITEMP1 FAILREG DSPTAB FLAGWRD3 FLAGWRD7 FLGWRD10 -PHASE4 BRUPT
TEMK SQ
ITEMP2 ITEMP3 FLAGWRD0 FLAGWRD4 DELAYLOC EXTVBACT
DNLSTCOD DUMPCNT LDATALST DNTMGOTO TMINDEX DNQ DNTMBUFF CTLIST DNECADR SUBLIST DUMPLOC
ERCOUNT REDOCTR DSRUPTSW ERESTORE SMODE UPSVFLAG NVSAVE EBANKTEM RATEINDX IMODES30 IMODES33 RESTREG
-PHASE1 -PHASE2 -PHASE3 -PHASE5 -PHASE6 RSBBQ SKEEP4 SKEEP5 SKEEP7 FLAGWRD1 FLAGWRD5 FLAGWRD6
FLAGWRD8 TIME4 TIME5 R1SAVE INLINK DSPCNT CADRSTOR REQRET CLPASS DSPLOCK MONSAVE MONSAVE1 VERBREG
NOUNREG DSPLIST MARKSTAT OPTCADR RADCADR ATTCADR LGYRO NOUT DSPCOUNT
CHAN5 CHAN6 CHAN12 CHAN14 CHAN33 DSALMOUT NAVKEYIN MNKEYIN
MMNUMBER MINDEX TEMPFLSH PHSPRDT2 PHSPRDT3 PHSPRDT4 TBASE4 FLAGWRD0 FLAGWRD2 FLAGWRD7
ITEMP1 ITEMP2 VAC5 NEWLOC NEWPRIO CYR
KEYTEMP1 KEYTEMP2 TIME6 DSRUPTEM CHAN32 OUT0'''.split()
# Erasable names one mission's listing has and the other's not.
ERASABLE_ONE = '''LOCALARM BANKALRM OPTMODES
THRUST RCSFLAGS ABDELV CH5MASK CH6MASK PVALVEST AGSWORD DB DAPBOOLS STIKSENS -RATEDB HIASCENT DKTRAP
LMTRAP DKKAOSN LMKAOSN LMOMEGAN DKOMEGAN DKDB RADMODES T6NEXT NXT6ADR NEXTP T5ADR SAMPLIM
WTOPTION TRKMKCNT VHFCNT OPTIND T5LOC RASFLAG
OUTROUTE FLGWRD11 MASSTMP CSMMASS FLAGWRD9 MMTEMP BASETEMP
T6LOC T6FURTHA NEXTU NEXTV'''.split()
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
C5/2 C3/2 C1/2
OCT77770 DELAYNUM TCSLEEP WAKECAD OCT40200
NEG0 ERASID PRIO34 MINB12 LOWIDCOD MINTIME2 DNADRDCR MINB1314 LDNPHAS2 LDNDUMP LDNDUMP1
PRIO12 BITS4&6 IM30INIF MAXDB BOOLSTRT EBANK6 STIKSTRT RATESTRT FULLAPS 77001OCT 60DEC IM33INIT
REFSMBIT SURFFBIT CMOONBIT LMOONBIT APSFLBIT HI5 BIT4 INTFLBIT 9,6,4 IFAILINH IM30INIR ENGONBIT
RSFLGBTS NUMGRPS DIDFLBIT RODFLBIT P21FLBIT RACTCADR -ELR LDNPHAS1 RMODINIT ACCSOKAY OCT30001
READRBIT NR29FBIT OCT32001 OCT27470 NORRMBIT R77FLBIT OCT74160 STARTEB VAC1ADRC LTHVACA NOUTCON
LESCHK VD1 OCT33 BIT3 OPTINITF OPTINITR NOP01BIT OCT6200 INTMASK .5SEC EBANK3 OCT37 OCT77603
OCT74777 PRIO25 SKIPVBIT SWINIT SWINIT+1 SWINIT+2 SWINIT+3 SWINIT+4 SWINIT+5 SWINIT+6 SWINIT+7
SWINIT+8 SWINIT+9 SWINIT+10 SWINIT+11
V37N99 DEC70 NOV37MM V37RETAD RNDVZBIT P25FLBIT OCT37667 OCT40072 R00AD LRBYBIT PRIO5 OCT700
DNLADP00 OCT120 OCT26 OCT500 OCT24 P20REG OCT305 V37QCAD PRIO37 3.1SEC AVEGBIT OCT44571 OCT600
BIT7-8 OCT01120 MAXDBBIT ELEVEN TWELVE EIGHTEEN NEG2 LOW7 TWO ZERO ONE BIT8 OCT31
LOW5 CHRPRIO HI10 ELRCODE DSKYFBIT UPLOCBIT
GOBB T3RPTBB KEYRPTBB MKRUPTBB UPRPTBB DWNRPTBB RDRPTBB T4RPTBB RUPT10BB HCRUPTBB 00314OCT
TABLNTH 120MRUPT 20MRUPT 14,11,9 11,14,9 BIT9 OCT37737 RELTAB RELTAB+1 RELTAB+2 RELTAB+3 RELTAB+4
RELTAB+5 RELTAB+6 RELTAB+7 RELTAB+8 RELTAB+9 RELTAB+10 RELTAB11'''.split()
WORDS2 = labels.WORDS2 + '''ADRPCHN2 600SECS IDLEADR T5IDLER T5IDLER+2 T5IDLER+4
T5IDLER+6 P70CADR T6ADR'''.split()
# The programs verb 37 starts (FRESH_START_AND_RESTART.agc: FCADRMM1's
# FCADRs), as FCADRs (F_<name>): the bank in bits 15-11, the address in
# the bank in bits 10-1.
FCADR = re.compile(r'^\S*\s+FCADR\s+(\S+)', re.M)


def fcadr_names(program):
    text = (PORT / f'build/fixed-data/{program}/FRESH_START_AND_RESTART.agc').read_text()
    start = text.index('\nFCADRMM1')
    return unique(FCADR.findall(text[start:text.index('PREMM1', start)]))


def fcadr(physical):
    """The FCADR of a fixed word: as an F-bank and the address in it
    (banks 040-043 show as 030-033, the superbank bit being separate)."""
    bank, offset = divmod(physical, 1024)
    if bank >= 0o40:
        bank -= 0o10
    return (bank << 10) | offset
# The operands of the interpretive code readable/ writes in listing
# notation (STATEUP, R00's CALL INTSTALL), as the interpreter reads them
# (I_<name>): a switch's bit number; an erasable word's address, under its
# EBANK (EBANK * 0400 + its address in the bank) in banks 3-7; a fixed
# word's bank * 02000 + its address in the bank (banks 040-043 as 030-033).
INTERPRETIVE = '''VINTFLAG SURFFLAG ORBWFLAG RENDWFLG DIM0FLAG D6OR9FLG PRECIFLG NODOFLAG
TETCSM TDEC1 STATEUP DOINT NO-INT DOINT2 STATEND INTEGRV INTSTALL SETIFLGS ENDINT'''.split()
# The erasable words the downlink lists name (DOWNLINK_LISTS.agc: the
# operands of its DNADRs), as ECADRs (E_<name>): the EBANK bits and the
# address in the bank, as a DNADR holds them.
DNADR = re.compile(r'^\S*\s+-?[1-6]DNADR\s+([A-Za-z0-9./*+()-]+)', re.M)


def downlist_names(program):
    text = open(READABLE.parent / program / 'DOWNLINK_LISTS.agc', errors='replace').read()
    return unique(DNADR.findall(text))


ECADR = re.compile(r'\d+,([EC]):\s+(\S+)\s+(?:E([0-7]),)?([0-7]+)\s')


def ecadrs(path):
    """{name: ECADR} of the erasable names in the listing's symbol table."""
    text = open(path, errors='replace').read()
    out = {}
    for kind, name, bank, value in ECADR.findall(text[text.index('Symbol Table'):] + '\n'):
        value = int(value, 8)
        if kind == 'C' and value >= 0o2000:
            continue
        if bank and value >= 0o1400:
            value = int(bank) * 0o400 + value - 0o1400
        out.setdefault(name, value)
    return out


def e_spelling(name):
    """The C spelling of a name in E_<name>: '-', '.', '/' and '(' as '_', no ')'
    (R-OTHER -> R_OTHER, LAT(SPL) -> LAT_SPL)."""
    return re.sub(r'[-./(]', '_', name.replace(')', ''))


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
    name = re.sub(r'(?<=\w)-(?=\w)', '_', name)  # NO-INT -> NO_INT; -ELR stays MELR
    return labels.macro(name.replace('*', '_STAR').replace('=', '_EQ_').replace(',', '_')
                        .replace('&', '_'))


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
    table = ecadrs(path)
    for n in downlist_names(program):
        if n not in table:
            raise SystemExit('%s: downlist name %s not in the symbol table' % (program, n))
        out.append('#define E_%s 0%04ou' % (e_spelling(n), table[n]))
    for n in fcadr_names(program):
        if where(n) is None:
            raise SystemExit('%s: program %s not in the symbol table' % (program, n))
        out.append('#define F_%s 0%05ou' % (macro(n), fcadr(where(n))))
    for n in INTERPRETIVE:
        if where(n) is not None:
            out.append('#define I_%s 0%05ou' % (macro(n), fcadr(where(n))))
        elif n in table:
            out.append('#define I_%s 0%05ou' % (macro(n), table[n]))
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
