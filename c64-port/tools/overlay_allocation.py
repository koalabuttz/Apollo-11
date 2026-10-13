#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-program module allocation plan for overlay ABI 2.

For every Luminary099 and Comanche055 source file this records the owning
lane, the placement class (resident, task slot, job slot, linked library,
data, declarations), an estimate of its AGC fixed-memory words, and the
native bytes and module count that estimate implies at low/mid/high
expansion. It is a planning budget, not coverage: port_coverage.py remains
the completion inventory, and a lane replaces an estimate with measured
module sizes as its modules land (OVERLAY_ABI_V2.md, "Allocation plan").

Word estimates come from a line classifier over the unchanged sources: basic
instructions and data words count one, double-word forms two, interpretive
lines count half a word per opcode plus one per operand. Both totals land
within a few percent of the 36,864-word fixed memory both programs nearly
fill.
"""
import argparse
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
MISSIONS = ('Luminary099', 'Comanche055')
# The linked flight resident (Resident kernel lane, resident_flight.ld): measured
# bytes per mission. The projection above budgets the files placed resident;
# this is what the linker actually produced. A PR that changes it reruns this
# script so docs/overlay-allocation.json stays current.
RESIDENT_MEASURED = ROOT / 'evidence/resident-kernel/resident-flight-sizes.json'
MEASURED_KEY = dict(Luminary099='lm', Comanche055='cm')

# Native bytes per AGC word for modules. Low comes from PHASCHNG (~131 words,
# 2,169 bytes in cm-clock-display-overlay-2.map). High (40) was once read off
# p66-resident-outlined.map, but about 80% of that "exec_*" text is the
# trampoline and the IRQ queue; keep it as a module-store bound only.
EXPANSION = dict(low=16, mid=25, high=40)
# The resident is the thin kernel (David, 2026-10-13): the certified
# exec-closure routines measure 92 B/word (54-56 KB per mission), too big for
# the resident; the thin kernel (executive.c + waitlist.c, 7.6 KB for ~612
# words) measures 12-19 B/word.
RESIDENT_EXPANSION = dict(low=12, mid=16, high=20)
DATA_BYTES_PER_WORD = 2.5
SLOT_FILL = 0.75              # usable fraction of a slot after seams and helpers
SLOT_BYTES = dict(job=0x4000, task=0x1FFA)
# Resident code and data below the $6F00 service table.
RESIDENT_BYTES = 0x6F00 - 0x0801
# ABI 2 runtime and REU store (measured 6,110 without the boot-only
# apollo_reu_store_verify), exported services (749), IRQ entry and device
# adapters (estimate). The disk loader (2,630) and REU verify (1,965) run at
# boot only and are not resident.
RESIDENT_PLATFORM = 6110 + 749 + 3000
# The 2 MB REU's top 256 KB holds both missions' rope images (Luminary099 at
# 0x1c0000, Comanche055 at 0x1e0000; include/rope_image.h), so the module
# store must end below it even at the high estimate.
REU_STORE_LIMIT = 0x1c0000

NOWORD = {'EQUALS', '=', 'ERASE', 'SETLOC', 'BANK', 'COUNT', 'COUNT*', 'BLOCK', 'EBANK=', 'SBANK=',
          'MEMORY', 'SUBRO', 'BNKSUM', '=MINUS', '=ECADR', 'CHECK=', 'EQUALS*'}
ONE = {'DEC', 'OCT', 'CADR', 'ADRES', 'GENADR', 'FCADR', 'ECADR', 'BBCON', 'REMADR', 'CAF', 'CA', 'TC',
       'TCF', 'CS', 'AD', 'TS', 'INDEX', 'EXTEND', 'INHINT', 'RELINT', 'CCS', 'XCH', 'LXCH', 'QXCH', 'MP',
       'MASK', 'SU', 'BZF', 'BZMF', 'INCR', 'ADS', 'AUG', 'DIM', 'WRITE', 'READ', 'ROR', 'WOR', 'RAND',
       'WAND', 'RXOR', 'EDRUPT', 'RESUME', 'RETURN', 'NOOP', 'DOUBLE', 'COM', 'ZL', 'ZQ', 'SQUARE',
       'DDOUBL', 'OVSK', 'TCAA', 'CAE', 'CAD', 'NDX', 'DNCHAN', 'DNPTR', '1DNADR', 'VN', 'MSU', 'MSK',
       'SR', 'CYR', 'CYL', 'EDOP'}
TWO = {'2DEC', '2DEC*', '2OCT', '2CADR', '2FCADR', '2BCADR', 'DXCH', 'DCA', 'DCS', 'DAS', 'DTCB',
       'DTCF', 'DV', '2DNADR', '3DNADR', '4DNADR', '5DNADR', '6DNADR'}


def estimate_words(path):
    words = 0.0
    for line in path.read_text(errors='replace').splitlines():
        code = line.split('#', 1)[0].rstrip()
        if not code.strip():
            continue
        fields = code.split()
        if not code[0].isspace():
            fields = fields[1:]
        if not fields or fields[0] in NOWORD:
            continue
        op = fields[0]
        if op in ONE:
            words += 1
        elif op in TWO:
            words += 2
        elif re.fullmatch(r'[A-Z0-9*,.+-]+', op):
            words += 0.5 * min(len(fields), 2)
            if len(fields) >= 2 and not re.fullmatch(r'[A-Z][A-Z0-9]*', fields[1]):
                words += 0.5
        else:
            words += 1
    return round(words)


def key(name):
    return re.sub(r'[^A-Z0-9]', '', name.upper().removesuffix('.AGC'))


LANES = {
    'A': ['INTERPRETER', 'EXECUTIVE', 'WAITLIST', 'FRESHSTARTANDRESTART', 'RESTARTSROUTINE',
          'RESTARTTABLES', 'PHASETABLEMAINTENANCE', 'T4RUPTPROGRAM', 'T6RUPTPROGRAMS',
          'INTERRUPTLEADINS', 'KEYRUPTUPRUPT', 'MAIN', 'ALARMANDABORT', 'UPDATEPROGRAM',
          'DOWNTELEMETRYPROGRAM', 'DOWNLINKLISTS', 'AGCBLOCKTWOSELFCHECK', 'INTERBANKCOMMUNICATION',
          'SINGLEPRECISIONSUBROUTINES', 'FIXEDFIXEDCONSTANTPOOL', 'INTERPRETIVECONSTANT',
          'INTERPRETIVECONSTANTS', 'RTBOPCODES', 'RT8OPCODES', 'SYSTEMTESTSTANDARDLEADINS',
          'SERVICEROUTINES'],
    'B': ['CONICSUBROUTINES', 'ORBITALINTEGRATION', 'INTEGRATIONINITIALIZATION', 'TIMEOFFREEFALL',
          'STABLEORBIT', 'PLANETARYINERTIALORIENTATION', 'LATITUDELONGITUDESUBROUTINES',
          'LUNARANDSOLAREPHEMERIDESSUBROUTINES', 'POWEREDFLIGHTSUBROUTINES', 'MYSUBS'],
    'C': ['PINBALLGAMEBUTTONSANDLIGHTS', 'PINBALLNOUNTABLES', 'EXTENDEDVERBS',
          'DISPLAYINTERFACEROUTINES'],
    'D1': ['IMUCALIBRATIONANDALIGNMENT', 'IMUCOMPENSATIONPACKAGE', 'IMUPERFORMANCETESTS4',
           'IMUPERFORMANCETEST2', 'IMUMODESWITCHINGROUTINES', 'INFLIGHTALIGNMENTROUTINES', 'P51P53',
           'SXTMARK', 'AOTMARK', 'R6062', 'KALCMANUSTEERING', 'ATTITUDEMANEUVERROUTINE', 'LEMGEOMETRY',
           'CSMGEOMETRY', 'ANGLFIND', 'GIMBALLOCKAVOIDANCE', 'SBANDANTENNAFORLM', 'SBANDANTENNAFORCM',
           'STARTABLES', 'LUNARLANDMARKSELECTIONFORCM'],
    'D2': ['P20P25', 'MEASUREMENTINCORPORATION', 'AOSTASKANDAOSJOB', 'RADARLEADINROUTINES',
           'KALMANFILTER'],
    'E': ['P30P37', 'P32P35P72P75', 'P32P33P72P73', 'P3435P7475', 'P37P70', 'P76', 'R30', 'R31',
          'TPISEARCH', 'GENERALLAMBERTAIMPOINTGUIDANCE', 'GROUNDTRACKINGDETERMINATIONPROGRAM'],
    'F': ['SERVICER', 'LUNARLANDINGGUIDANCEEQUATIONS', 'THELUNARLANDING',
          'BURNBABYBURNMASTERIGNITIONROUTINE', 'P12', 'P70P71', 'R63', 'ASCENTGUIDANCE',
          'THROTTLECONTROLROUTINES', 'PAXISRCSAUTOPILOT', 'QRAXISRCSAUTOPILOT', 'TJETLAW',
          'TRIMGIMBALCONTROLSYSTEM', 'DAPIDLERPROGRAM', 'DAPINTERFACESUBROUTINES',
          'FINDCDUWGUIDAPINTERFACE', 'LANDINGANALOGDISPLAYS', 'AGSINITIALIZATION',
          'RCSFAILUREMONITOR', 'SPSBACKUPRCSCONTROL'],
    'G': ['SERVICER207', 'P11', 'P61P67', 'CMENTRYDIGITALAUTOPILOT', 'REENTRYCONTROL',
          'RCSCSMDAPEXECUTIVEPROGRAMS', 'RCSCSMDIGITALAUTOPILOT', 'JETSELECTIONLOGIC', 'TVCDAPS',
          'TVCEXECUTIVE', 'TVCINITIALIZE', 'TVCMASSPROP', 'TVCRESTARTS', 'TVCROLLDAP', 'TVCSTROKETEST',
          'AUTOMATICMANEUVERS', 'CMBODYATTITUDE', 'ENTRYLEXICON'],
    'H': ['ERASABLEASSIGNMENTS', 'FLAGWORDASSIGNMENTS', 'INPUTOUTPUTCHANNELBITDESCRIPTIONS',
          'TAGSFORRELATIVESETLOC', 'ASSEMBLYANDOPERATIONINFORMATION', 'CONTRACTANDAPPROVALS',
          'CONTROLLEDCONSTANTS'],
}
# P40-P47 exists in both programs: LM powered flight is lane F, CM is lane G.
MISSION_LANE = {('Luminary099', 'P40P47'): 'F', ('Comanche055', 'P40P47'): 'G'}

PLACEMENT = {
    # Always resident: the trampoline itself, interrupt entry, bank-call
    # glue and the routines every slot calls with interrupts inhibited.
    'resident': ['EXECUTIVE', 'WAITLIST', 'SINGLEPRECISIONSUBROUTINES'],
    'resident-data': ['FIXEDFIXEDCONSTANTPOOL'],
    # Replaced by the ABI 2 mechanism itself (continuation resolver and
    # trampoline, native IRQ entry); budgeted in RESIDENT_PLATFORM.
    'mechanism': ['INTERBANKCOMMUNICATION', 'INTERRUPTLEADINS'],
    # Interrupt-level (T3/T4/T5/T6/KEYRUPT/DOWNRUPT) bodies: task slot.
    'task': ['PHASETABLEMAINTENANCE', 'T4RUPTPROGRAM', 'T6RUPTPROGRAMS', 'KEYRUPTUPRUPT', 'DOWNTELEMETRYPROGRAM',
             'PAXISRCSAUTOPILOT', 'QRAXISRCSAUTOPILOT', 'TJETLAW', 'TRIMGIMBALCONTROLSYSTEM',
             'DAPIDLERPROGRAM', 'RCSFAILUREMONITOR', 'RCSCSMDIGITALAUTOPILOT', 'JETSELECTIONLOGIC',
             'TVCDAPS', 'TVCROLLDAP', 'CMENTRYDIGITALAUTOPILOT', 'IMUCOMPENSATIONPACKAGE'],
    # Interpretive arithmetic: native helpers linked into the modules that
    # use them, with a hot core exported through the resident service table.
    'library': ['INTERPRETER', 'RTBOPCODES', 'RT8OPCODES'],
    'data': ['CONTROLLEDCONSTANTS', 'INTERPRETIVECONSTANT', 'INTERPRETIVECONSTANTS',
             'PINBALLNOUNTABLES', 'DOWNLINKLISTS', 'STARTABLES', 'RESTARTTABLES'],
    'declarations': ['ERASABLEASSIGNMENTS', 'FLAGWORDASSIGNMENTS', 'INPUTOUTPUTCHANNELBITDESCRIPTIONS',
                     'TAGSFORRELATIVESETLOC', 'ASSEMBLYANDOPERATIONINFORMATION',
                     'CONTRACTANDAPPROVALS', 'ENTRYLEXICON'],
}


def lane_of(mission, k):
    if (mission, k) in MISSION_LANE:
        return MISSION_LANE[(mission, k)]
    for lane, names in LANES.items():
        if k in names:
            return lane
    raise SystemExit(f'{mission}/{k}: no lane assigned')


def placement_of(k):
    for placement, names in PLACEMENT.items():
        if k in names:
            return placement
    return 'job'


def native_bytes(words, placement):
    if placement in ('declarations', 'mechanism'):
        return {level: 0 for level in EXPANSION}
    if placement in ('data', 'resident-data'):
        return {level: round(words * DATA_BYTES_PER_WORD) for level in EXPANSION}
    factors = RESIDENT_EXPANSION if placement == 'resident' else EXPANSION
    return {level: words * factor for level, factor in factors.items()}


def plan():
    missions = []
    for mission in MISSIONS:
        files = []
        for path in sorted((ROOT.parent / mission).glob('*.agc')):
            k = key(path.name)
            words = estimate_words(path)
            placement = placement_of(k)
            files.append(dict(path=f'{mission}/{path.name}', lane=lane_of(mission, k),
                              placement=placement, estimated_words=words,
                              estimated_native_bytes=native_bytes(words, placement)))
        totals = {}
        for f in files:
            t = totals.setdefault(f['placement'], dict(files=0, words=0, bytes={l: 0 for l in EXPANSION}))
            t['files'] += 1; t['words'] += f['estimated_words']
            for level in EXPANSION:
                t['bytes'][level] += f['estimated_native_bytes'][level]
        lanes = {}
        for f in files:
            l = lanes.setdefault(f['lane'], dict(files=0, words=0, job_modules_mid=0.0, task_modules_mid=0.0))
            l['files'] += 1; l['words'] += f['estimated_words']
            if f['placement'] in ('job', 'data'):
                l['job_modules_mid'] += f['estimated_native_bytes']['mid'] / (SLOT_BYTES['job'] * SLOT_FILL)
            elif f['placement'] == 'task':
                l['task_modules_mid'] += f['estimated_native_bytes']['mid'] / (SLOT_BYTES['task'] * SLOT_FILL)
        for l in lanes.values():
            l['job_modules_mid'] = -(-l['job_modules_mid'] // 1)
            l['task_modules_mid'] = -(-l['task_modules_mid'] // 1)
        # Library helpers are linked into consumers; budget them once more
        # across job modules at the mid estimate.
        store = {level: sum(totals.get(p, {'bytes': {level: 0}})['bytes'][level]
                            for p in ('job', 'task', 'data', 'library')) for level in EXPANSION}
        resident = {level: sum(totals.get(p, {'bytes': {level: 0}})['bytes'][level]
                               for p in ('resident', 'resident-data')) + RESIDENT_PLATFORM
                    for level in EXPANSION}
        measured = None
        if RESIDENT_MEASURED.exists():
            m = json.loads(RESIDENT_MEASURED.read_text()).get(MEASURED_KEY[mission])
            if m:
                measured = dict(source=str(RESIDENT_MEASURED.relative_to(ROOT)),
                                resident_bytes=m['resident_bytes'], free_bytes=m['free_bytes'],
                                boot_only_bytes=m.get('boot_only_bytes'), components=m.get('components', {}))
        missions.append(dict(mission=mission, total_words=sum(f['estimated_words'] for f in files),
                             placements=totals, lanes=lanes,
                             store_bytes=store, resident_bytes=resident,
                             resident_capacity=RESIDENT_BYTES, resident_measured=measured, files=files))
    return dict(schema_version=1, abi=2,
                expansion_bytes_per_word=EXPANSION, resident_bytes_per_word=RESIDENT_EXPANSION,
                data_bytes_per_word=DATA_BYTES_PER_WORD,
                slot_bytes=SLOT_BYTES, slot_fill=SLOT_FILL, reu_store_limit=REU_STORE_LIMIT,
                lane_module_id_high_byte=dict(A=1, B=2, C=3, D1=4, D2=5, E=6, F=7, G=8, H=9, proof=15),
                missions=missions)


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--output', type=Path, default=ROOT / 'docs/overlay-allocation.json')
    args = p.parse_args()
    result = plan()
    args.output.write_text(json.dumps(result, indent=1) + '\n')
    for m in result['missions']:
        kb = {l: round(v / 1024) for l, v in m['store_bytes'].items()}
        rk = {l: round(v / 1024, 1) for l, v in m['resident_bytes'].items()}
        print(f"{m['mission']}: {m['total_words']} words; module store {kb} KB; "
              f"resident {rk} KB of {m['resident_capacity'] / 1024:.1f} KB")
        if m['resident_measured']:
            r = m['resident_measured']
            print(f"  linked resident {r['resident_bytes']} B, {r['free_bytes']} B free ({r['source']})")
        for placement, t in sorted(m['placements'].items()):
            print(f"  {placement:13s} {t['files']:3d} files {t['words']:6d} words  mid {t['bytes']['mid'] / 1024:7.1f} KB")
        for lane, l in sorted(m['lanes'].items()):
            print(f"  lane {lane:3s} {l['files']:3d} files {l['words']:6d} words  ~{int(l['job_modules_mid'])} job + {int(l['task_modules_mid'])} task modules (mid)")
    over = [m['mission'] for m in result['missions'] if m['store_bytes']['high'] > REU_STORE_LIMIT]
    if over:
        raise SystemExit(f"{', '.join(over)}: high module store estimate crosses the "
                         f"reserved rope images at REU {REU_STORE_LIMIT:#x}")


if __name__ == '__main__':
    main()
