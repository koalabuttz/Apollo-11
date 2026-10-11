#!/usr/bin/env python3
"""Compare native outputs with ORIGINAL Executive running in Virtual AGC."""
import argparse
import hashlib
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--reference', required=True, type=pathlib.Path,
               help='prepared fixture directory (contains MAIN.agc.bin)')
p.add_argument('--virtualagc', type=pathlib.Path,
               help='Virtual AGC checkout; default: fixture sibling virtualagc')
a = p.parse_args()
reference = a.reference.resolve()
virtual = (a.virtualagc or reference.parent / 'virtualagc').resolve()
engine = virtual / 'yaAGC'
original = ROOT.parent / 'Luminary099/EXECUTIVE.agc'
if original.read_bytes() != (reference / 'EXECUTIVE.agc').read_bytes():
    sys.exit('FAIL: fixture Executive is not byte-identical to upstream')
subprocess.run(['gcc', '-std=c99', '-O2', '-I'+str(engine), '-Iinclude', '-Itests',
                'tools/reference_runner.c', 'tests/scenarios.c',
                str(engine/'agc_engine.o'), str(engine/'agc_engine_init.o'),
                str(engine/'rfopen.o'), '-o', 'build/reference-runner'], cwd=ROOT, check=True)
native = subprocess.run(['./build/snapshot'], cwd=ROOT, capture_output=True, text=True, check=True)
oracle = subprocess.run(['./build/reference-runner', str(reference/'MAIN.agc.bin')],
                        cwd=ROOT, capture_output=True, text=True, check=True)
(ROOT/'build/native.snapshots').write_text(native.stdout)
(ROOT/'build/reference.snapshots').write_text(oracle.stdout)
(ROOT/'build/reference.trace').write_text(oracle.stderr)
nr, rr = native.stdout.splitlines(), oracle.stdout.splitlines()
if len(nr) != 26 or len(rr) != 26:
    sys.exit('FAIL: expected exactly 26 output scenarios')
errors=[]
for n,(native_row,reference_row) in enumerate(zip(nr,rr)):
    for k,(nv,rv) in enumerate(zip(native_row.split(),reference_row.split())):
        if nv != rv:
            errors.append(f'scenario {n}, field {k}: native={nv}, reference={rv}')
    if len(native_row.split()) != len(reference_row.split()):
        errors.append(f'scenario {n}: row length mismatch')
if errors:
    print('\n'.join(errors)); sys.exit(1)
print('PASS: 26 original-assembly scenarios; 323 AGC state words plus alarm and status per scenario agree')
print('Boundaries: ENDFIND / BAILOUT1 / CHANJOB-2 / DUMMYJOB entry; no restart or dispatch claim')
native_arith=subprocess.run(['./build/arithmetic-dump'],cwd=ROOT,capture_output=True,check=True).stdout
reference_arith=subprocess.run(['./build/reference-runner','--arithmetic'],cwd=ROOT,capture_output=True,check=True).stdout
if len(native_arith)!=1245184 or native_arith!=reference_arith:
    sys.exit('FAIL: Virtual AGC arithmetic helper comparison')
print('PASS: 32768 sign extensions, 65536 overflow corrections, 524288 accumulator additions against Virtual AGC helper API')
print('Arithmetic binary stream SHA256: '+hashlib.sha256(reference_arith).hexdigest())

subprocess.run(['gcc','-std=c99','-O2','-DDISPATCH_INPUT_ONLY','-I'+str(engine),
                '-Iinclude','-Itests','tools/dispatch_reference.c','tests/scenarios.c',
                'tests/dispatch_cases.c',str(engine/'agc_engine.o'),
                str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/dispatch-reference'],
               cwd=ROOT,check=True)
outputs=[]
for name,command in [('native',['./build/dispatch-snapshot']),
                     ('reference',['./build/dispatch-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,check=True)
    (ROOT/('build/dispatch-'+name+'.snapshots')).write_text(run.stdout)
    if name=='reference': (ROOT/'build/dispatch-reference.trace').write_text(run.stderr)
    outputs.append([list(map(int,row.split())) for row in run.stdout.splitlines()])
if any(len(rows)!=109 for rows in outputs): sys.exit('FAIL: expected 109 dispatch checkpoints')
errors=[]
for row,(native_row,reference_row) in enumerate(zip(*outputs)):
    if len(native_row)!=329 or len(reference_row)!=329:
        errors.append(f'dispatch row {row}: expected 329 fields')
    for field,(nv,rv) in enumerate(zip(native_row,reference_row)):
        if nv!=rv: errors.append(f'dispatch case/step {reference_row[:2]}, field {field}: native={nv}, reference={rv}')
if errors: sys.exit('\n'.join(errors))
print('PASS: 53 dispatch cases / 109 checkpoints, including 8 sequential two-job native executions')
print('Full saved state and selected 2CADR match original basic dispatch; BBANK and SUPERBNK checked independently')
print('Dispatch snapshot SHA256: '+hashlib.sha256((ROOT/'build/dispatch-reference.snapshots').read_bytes()).hexdigest())

subprocess.run(['gcc','-std=c99','-O2','-DDISPATCH_INPUT_ONLY','-DLIFECYCLE_INPUT_ONLY',
                '-I'+str(engine),'-Iinclude','-Itests','tools/lifecycle_reference.c',
                'tests/scenarios.c','tests/dispatch_cases.c','tests/lifecycle_cases.c',
                str(engine/'agc_engine.o'),str(engine/'agc_engine_init.o'),
                str(engine/'rfopen.o'),'-o','build/lifecycle-reference'],cwd=ROOT,check=True)
outputs=[]
for name,command in [('native',['./build/lifecycle-snapshot']),
                     ('reference',['./build/lifecycle-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,check=True)
    (ROOT/('build/lifecycle-'+name+'.snapshots')).write_text(run.stdout)
    if name=='reference': (ROOT/'build/lifecycle-reference.trace').write_text(run.stderr)
    outputs.append([list(map(int,row.split())) for row in run.stdout.splitlines()])
if any(len(rows)!=115 for rows in outputs): sys.exit('FAIL: expected 115 lifecycle checkpoints')
errors=[]
for row,(native_row,reference_row) in enumerate(zip(*outputs)):
    if len(native_row)!=329 or len(reference_row)!=329:
        errors.append(f'lifecycle row {row}: expected 329 fields')
    for field,(nv,rv) in enumerate(zip(native_row,reference_row)):
        if nv!=rv: errors.append(f'lifecycle case/step {reference_row[:2]}, field {field}: native={nv}, reference={rv}')
if errors: sys.exit('\n'.join(errors))
print('PASS: 81 sleep/wake/priority cases / 115 checkpoints, including 5 native lifecycle sequences')
print('Full saved state and actual bank-qualified transfer match original JOBSLEEP/JOBWAKE/PRIOCHNG')
print('Lifecycle snapshot SHA256: '+hashlib.sha256((ROOT/'build/lifecycle-reference.snapshots').read_bytes()).hexdigest())

for source in ['WAITLIST.agc','INTERRUPT_LEAD_INS.agc']:
    if (ROOT.parent/'Luminary099'/source).read_bytes() != (reference/source).read_bytes():
        sys.exit('FAIL: fixture '+source+' is not byte-identical to upstream')
subprocess.run(['gcc','-std=c99','-O2','-DWAIT_INPUT_ONLY','-I'+str(engine),
                '-Iinclude','-Itests','tools/waitlist_reference.c','tests/scenarios.c',
                'tests/waitlist_cases.c',str(engine/'agc_engine.o'),
                str(engine/'agc_engine_init.o'),str(engine/'rfopen.o'),'-o','build/waitlist-reference'],cwd=ROOT,check=True)
outputs=[]
for name,command in [('native',['./build/waitlist-snapshot']),
                     ('reference',['./build/waitlist-reference',str(reference/'MAIN.agc.bin')])]:
    run=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,check=True)
    (ROOT/('build/waitlist-'+name+'.snapshots')).write_text(run.stdout)
    if name=='reference': (ROOT/'build/waitlist-reference.trace').write_text(run.stderr)
    outputs.append([list(map(int,row.split())) for row in run.stdout.splitlines()])
if any(len(rows)!=306 for rows in outputs): sys.exit('FAIL: expected 306 WAITLIST checkpoints')
errors=[]
for row,(native_row,reference_row) in enumerate(zip(*outputs)):
    if len(native_row)!=34 or len(reference_row)!=34: errors.append(f'WAITLIST row {row}: expected 34 fields')
    for field,(nv,rv) in enumerate(zip(native_row,reference_row)):
        if nv!=rv: errors.append(f'WAITLIST case/step {reference_row[:2]}, field {field}: native={nv}, reference={rv}')
if errors: sys.exit('\n'.join(errors))
print('PASS: 196 WAITLIST cases / 306 checkpoints, including 10 actual native task sequences')
print('TIME3/LST1/LST2/RUPTAGN, retained 1203/1204 boundaries and physical task transfers match original assembly')
print('WAITLIST snapshot SHA256: '+hashlib.sha256((ROOT/'build/waitlist-reference.snapshots').read_bytes()).hexdigest())

subprocess.run([sys.executable,'tools/restart_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/guidance_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/prepare_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/scale_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/push_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/elapsed_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/store_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/divide_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/gravity_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/vector_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/executive_service_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/phase_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/product_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/shift_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dp_divide_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/velocity_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/norm_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/recovery_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dot_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/sum_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/interpreter_native_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/enema_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_executive_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/unit_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/radial_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/acceleration_prefix_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/acceleration_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/waitlist_zero_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_dispatch_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_startup_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/trig_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_native_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/rotation_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/projection_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/projection_prefix_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/hardware_divide_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_initialization_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/throttle_feedback_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/throttle_service_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_bias_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_vertical_start_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_waitlist_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_irq_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_major_mode_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_launch_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/horizontal_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_mode_display_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/throttle_bridge_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_phase_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/cm_restart_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_restart_native_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_enema_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/unit_phased_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/radial_phased_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/projection_phased_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_relay_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_startup_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/acceleration_phased_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/acceleration_control_differential.py'],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/velocity_phased_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/throttle_feedback_phased_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/display_control_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/regodspr_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/acceleration_store_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/nv_entry_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/nv_words_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_pinball_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_decimal_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_scale_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_two_digit_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_numeric_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_rotation_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_rectify_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_difeq_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_orbital_initial_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_pinball_zero_differential.py',str(reference)],cwd=ROOT,check=True)
subprocess.run([sys.executable,'tools/p66_mode_zero_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/nv_rows_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/nv_render_full_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/nv_render_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,"tools/goplay_full_differential.py",str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/goplay_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/lm_priority_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/runtime_bridge_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/p66_fine_phases_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,"tools/p66_basic_task_differential.py",str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_nv_key_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_charin_nv_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_keycom_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/dsky_keyrupt_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_tig5_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_entry_tasks_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/runtime_regodspr_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/runtime_fetch_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/lm_wake_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/makeplay_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_restart_timing_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_longcall_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_restart_select_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_clocktask_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_waitlist_image_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_task_transfer_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_admission_image_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_svct3_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_delay_image_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_trig_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/cm_single_return_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/lm_sleep_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/makeplay_sleep_differential.py',str(reference)],cwd=ROOT,check=True)

subprocess.run([sys.executable,'tools/lm_busy_differential.py',str(reference)],cwd=ROOT,check=True)
