#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_scenario_vice.py --replay: host and C64 states paired by
centisecond, the C64 end a centisecond after the host's included."""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_scenario_vice as fsv  # noqa: E402
import program_acceptance as pa  # noqa: E402

N = fsv.STATE // 2 - 1          # erasable words and output channels in a state
WORD = next(i for i in range(0o100, 2048) if fsv.compared(i) and i not in fsv.TIMERS)


def state(cs, **set_):
    """A state at cs: TIME1 (025) and TIME3 (026) count with the window's
    centiseconds, everything else 0 except the words given (octal names)."""
    w = [0] * N
    w[0o25], w[0o26] = cs, 0o37000 + cs
    for k, v in set_.items():
        w[int(k[1:], 8)] = v
    return (cs, w)


class Pairing(unittest.TestCase):
    def test_same_cs_ends_pair(self):
        v = fsv.replay_verdict([state(50), state(100)], [state(50), state(100)], True, after=[state(101)])
        self.assertEqual((v['diff'], v['matched'], v['end']), ([], [50, 100], dict(shift_cs=0, paired=True)))

    def test_late_c64_end_pairs_with_the_hosts_state_there(self):
        # The host ends at 172; the C64's first sample point is 173. The
        # host's state at 173 (written past its end) is the same as the C64's.
        v = fsv.replay_verdict([state(100), state(172)], [state(100), state(173)], True,
                               after=[state(173), state(175)], after_rows=[])
        self.assertEqual(v['diff'], [])
        self.assertEqual(v['matched'], [100, 173])
        self.assertEqual(v['end'], dict(shift_cs=1, paired=True))
        self.assertEqual(v['unpaired'], [])

    def test_busy_c64_end_pairs_at_the_first_point_both_have(self):
        # The host ends at 170 and writes states at 171, 173 and 175; the C64
        # is busy until 172, then writes one at its next marks.
        v = fsv.replay_verdict([state(100), state(170)], [state(100), state(172), state(173), state(175)], True,
                               after=[state(171), state(173), state(175)])
        self.assertEqual(v['diff'], [])
        self.assertEqual(v['matched'], [100, 173])
        self.assertEqual(v['end'], dict(shift_cs=3, paired=True))
        self.assertEqual(v['unpaired'], [])

    def test_late_c64_end_with_a_data_difference_still_fails(self):
        v = fsv.replay_verdict([state(100), state(172)], [state(100), state(173, **{f'w{WORD:o}': 0o123})], True,
                               after=[state(173)], after_rows=[])
        self.assertEqual(v['end'], dict(shift_cs=1, paired=True))
        self.assertEqual(v['at_cs'], 173)
        self.assertEqual(v['diff'], [(f'{WORD:04o}', 0, 0o123)])

    def test_late_end_unpaired_when_an_input_comes_in_between(self):
        v = fsv.replay_verdict([state(172)], [state(173)], True, after=[state(173)], after_rows=[172])
        self.assertEqual(v['diff'], [])
        self.assertEqual(v['matched'], [])
        self.assertFalse(v['end']['paired'])
        self.assertTrue(v['end']['inputs_between'])
        self.assertEqual([(u['side'], u['cs']) for u in v['unpaired']], [('host', 172), ('C64', 173)])

    def test_late_end_unpaired_when_the_host_has_no_state_there(self):
        v = fsv.replay_verdict([state(172)], [state(174)], True, after=[state(173), state(176)], after_rows=[])
        self.assertEqual(v['end'], dict(shift_cs=2, paired=False, inputs_between=False, host_has_cs=False))

    def test_unpaired_end_is_never_a_match(self):
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100], ok=False,
                 replay_end_paired=False, replay_end_shift_cs=2, replay_unpaired_count=0, reu_bounds=dict(ok=True, dma_outside=0, dma_trace_armed=True))
        ok, text = pa.replay_result(r)
        self.assertFalse(ok)
        self.assertIn('end not compared', text)

    def test_shifted_match_is_shown(self):
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100, 173],
                 replay_end_paired=True, replay_end_shift_cs=1, replay_unpaired_count=0, reu_bounds=dict(ok=True, dma_outside=0, dma_trace_armed=True))
        self.assertEqual(pa.replay_result(r), (True, 'matches host (end compared 1 cs late)'))

    def test_unpaired_checkpoints_fail_the_result(self):
        # #766: a checkpoint with no partner was not compared, so not a match.
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100, 300],
                 replay_end_paired=True, replay_end_shift_cs=0, replay_unpaired_count=2, reu_bounds=dict(ok=True, dma_outside=0, dma_trace_armed=True))
        self.assertEqual(pa.replay_result(r), (False, '2 checkpoints not compared'))
        self.assertEqual(pa.replay_result({**r, 'replay_unpaired_count': None}),
                         (False, 'no record of unpaired checkpoints'))

    def test_reu_bounds_must_be_recorded_and_armed(self):
        # #766: no bounds record, or a DMA trace that was never armed, is not a pass.
        r = dict(status=0, replay_ended=True, replay_differences=0, replay_matched_cs=[100],
                 replay_end_paired=True, replay_end_shift_cs=0, replay_unpaired_count=0, reu_bounds=dict(ok=True, dma_outside=0, dma_trace_armed=True))
        self.assertEqual(pa.replay_result(r), (True, 'matches host'))
        self.assertEqual(pa.replay_result({k: v for k, v in r.items() if k != 'reu_bounds'}),
                         (False, 'no REU bounds result'))
        self.assertEqual(pa.replay_result({**r, 'reu_bounds': dict(ok=True, dma_outside=0)}),
                         (False, 'REU DMA trace not armed'))
        self.assertEqual(pa.replay_result({**r, 'reu_bounds': dict(ok=True, dma_outside=1, dma_trace_armed=True)}),
                         (False, 'REU access outside the layout'))

    def test_channel_10_and_interrupt_enable_are_compared(self):
        h, c = state(100)[1], state(100)[1]
        c[2048 + fsv.END_CHANNELS.index('channel 10')] = 0o40017
        c[2048 + fsv.END_CHANNELS.index('interrupts enabled')] = 1
        self.assertEqual([x[0] for x in fsv.replay_compare(h, c, None, set())], ['channel 10', 'interrupts enabled'])

    def test_chain_window_ends_always_pair(self):
        # A window before a chain's last has no .after states: its end is
        # the next window's seam and pairs as before (timers then differ).
        v = fsv.replay_verdict([state(320)], [state(321)], True, after=None)
        self.assertTrue(v['end']['paired'])
        self.assertEqual(v['diff'][0], ('centiseconds', 320, 321))

    def test_rows_before_counts_the_past_end_inputs_due(self):
        rows = b''.join(fsv.ROW.pack(cs, 1, 1, 0, 1, 0) for cs in (171, 172, 174))
        self.assertEqual([fsv.rows_before(rows, cs) for cs in (171, 172, 173, 175)], [0, 1, 2, 3])


class DeadSuperbank(unittest.TestCase):
    """The superbank at ADVAN (fsv.dead_superbank): masked and counted only
    when both sides sit at ADVAN with FB below 060000."""

    def pair(self, z=0o3214, fb=0o2000, c64_fb=None, c64_z=None, sb=(0o60, 0o100)):
        h, c = state(500)[1], state(500)[1]
        h[5], c[5] = z, z if c64_z is None else c64_z
        h[4], c[4] = fb, fb if c64_fb is None else c64_fb
        h[fsv.SUPERBANK], c[fsv.SUPERBANK] = sb
        return h, c

    def test_masked_at_advan(self):
        masked = {}
        h, c = self.pair()
        self.assertEqual(fsv.replay_compare(h, c, None, set(), 'lm', masked), [])
        self.assertEqual(masked, {fsv.DEAD_SUPERBANK: 1})
        h, c = self.pair(z=0o3231)
        self.assertEqual(fsv.replay_compare(h, c, None, set(), 'cm', masked), [])
        self.assertEqual(masked, {fsv.DEAD_SUPERBANK: 2})

    def test_verdict_counts_and_matches(self):
        h, c = self.pair()
        v = fsv.replay_verdict([(500, h)], [(500, c)], True, after=[], m='lm')
        self.assertEqual((v['diff'], v['matched'], v['masked'][fsv.DEAD_SUPERBANK]), ([], [500], 1))

    def test_not_masked_elsewhere(self):
        # FB not bank 01 on either side or both (banks 05 and 10 hold real
        # code at the same offset), Z at ADVAN on
        # neither side or one side only, the wrong mission or none.
        for kw, m in ((dict(fb=0o60000), 'lm'), (dict(fb=0o60000, c64_fb=0o2000), 'lm'), (dict(fb=0o12000), 'lm'),
                      (dict(z=0o3231, fb=0o20000), 'cm'), (dict(c64_fb=0o4000), 'lm'),
                      (dict(c64_fb=0o60000), 'lm'), (dict(z=0o3215), 'lm'), (dict(c64_z=0o3215), 'lm'),
                      (dict(z=0o3215, c64_z=0o3214), 'lm'), (dict(z=0o3214), 'cm'), ({}, None)):
            masked = {}
            h, c = self.pair(**kw)
            diff = fsv.replay_compare(h, c, None, set(), m, masked)
            self.assertIn(('superbank', 0o60, 0o100), diff, (kw, m))
            self.assertEqual(masked, {}, (kw, m))

    def test_only_selfbank_against_another(self):
        # One side must hold SELFBANK's 0100, either side; any other pair of
        # superbanks is a fault, not a dead read.
        for sb in ((0o100, 0o60), (0, 0o100), (0o100, 0)):
            self.assertEqual(fsv.replay_compare(*self.pair(sb=sb), None, set(), 'lm', {}), [], sb)
        for sb in ((0, 0o60), (0o60, 0o70)):
            self.assertEqual(fsv.replay_compare(*self.pair(sb=sb), None, set(), 'lm', {}), [('superbank', *sb)], sb)

    def test_other_channel_at_a_masked_pair(self):
        # Only the superbank is masked: another channel differing at the same
        # pair is reported, alone.
        h, c = self.pair()
        k = 2048 + fsv.END_CHANNELS.index('channel 11')
        c[k] = h[k] ^ 0o40
        masked = {}
        self.assertEqual(fsv.replay_compare(h, c, None, set(), 'lm', masked), [('channel 11', h[k], c[k])])
        self.assertEqual(masked, {fsv.DEAD_SUPERBANK: 1})

    def test_replay_exact_masks_nothing(self):
        h, c = self.pair()
        for exact, want in ((False, []), (True, [('superbank', 0o60, 0o100)])):
            self.assertEqual(fsv.replay_compare(h, c, None, set(), fsv.mask_mission(exact, 'lm'), {}), want, exact)

    def test_replay_exact_counts_time5_phase(self):
        # TIME5 a count apart is a noted skip normally, a difference when exact.
        h, c = self.pair(sb=(0o60, 0o60))
        c[0o30] = (h[0o30] + 1) % 0o40000
        skipped = set()
        self.assertEqual(fsv.replay_compare(h, c, None, skipped, 'lm', {}), [])
        self.assertEqual(skipped, {fsv.TIME5_PHASE})
        skipped = set()
        self.assertEqual(fsv.replay_compare(h, c, None, skipped, 'lm', {}, exact=True), [('TIME5', h[0o30], c[0o30])])
        self.assertEqual(skipped, set())

    def test_verdict_lists_masked_points(self):
        h, c = self.pair()
        v = fsv.replay_verdict([(500, h)], [(500, c)], True, after=[], m='lm')
        self.assertEqual(v['masked'][fsv.DEAD_SUPERBANK_AT], [(500, 0o60, 0o100)])

    def test_other_differences_still_count(self):
        h, c = self.pair()
        c[WORD] = 0o123
        self.assertEqual(fsv.replay_compare(h, c, None, set(), 'lm', {}), [(f'{WORD:04o}', 0, 0o123)])


if __name__ == '__main__':
    unittest.main()
