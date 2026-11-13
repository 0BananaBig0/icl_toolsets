#!/usr/bin/env python3
"""iTake/iRelease membership includes serial and clock ports without widening iWrite."""
import unittest

import test_shift_enable_clients as helpers
import test_multiple_client_interfaces as selected


class PortReservations(unittest.TestCase):
    setUp = helpers.ShiftEnableClients.setUp
    run_tool = helpers.ShiftEnableClients.run_tool
    replay = helpers.ShiftEnableClients.replay

    def test_unwritten_serial_controls_remain_available(self):
        ports = ('si', 'so0', 'shift', 'ce', 'ue', 'ck')
        body = 'iReset; ' + ''.join(f'iTake {port}; ' for port in ports)
        body += 'iWrite c0.value 9; iWrite c1.value 6; iApply; '
        body += ''.join(f'iRelease {port}; ' for port in ports)
        body += 'iRead c0.value 9; iRead c1.value 6; iApply;\n'
        self.replay(helpers.network(), body)

    def test_indexed_ports_and_instance_release(self):
        self.replay(helpers.network(), 'iReset; iTake si[0]; iTake shift[1]; '
                    'iTake c0; iWrite c0.value 9; iApply; iRelease c0; '
                    'iRelease si[0]; iRelease shift[1]; iWrite c1.value 6; iApply;\n')

    def test_select_reset_and_system_clock_membership(self):
        source = helpers.network().replace('Module Chains {', '''Module Chains {
 SelectPort select[1:0]; ResetPort reset;
 ClockPort system_clock; ToClockPort system_out { Source system_clock; }
''')
        for i in range(2):
            source = source.replace(f'Port shift[{i}];', f'Port select[{i}]; Port shift[{i}];')
        self.replay(source, 'iReset; iTake select; iTake reset; '
                    'iTake system_clock; iTake system_out; '
                    'iWrite c0.value 9; iApply; iRelease reset; '
                    'iRelease system_clock; iRelease system_out; iRelease select;\n')

    def test_nested_persistent_and_inherited_release(self):
        self.replay(helpers.network(), '''iPDLLevel 1
iProcsForModule Chains
iProc child {} { iTake shift[0]; iTake ck -persistent; iWrite c0.value 9; iApply; }
iProc parent {} { iTake si; iCall child; iRelease ck; }
iReset
iCall parent
iRelease shift[0]
iWrite c1.value 6; iApply
''')

    def test_serial_writes_still_fail_before_queue_discard(self):
        for port in ('si[0]', 'shift[0]', 'ce', 'ue', 'ck', 'so0'):
            with self.subTest(port=port):
                self.reject(helpers.network(), f'iTake {port}; iWrite {port} 0; iReset;\n')

    def test_taken_select_state_constrains_client_selection(self):
        source = selected.network()
        prefix = 'iReset; iTake sel[0]; iState sel[0] 1; '
        self.replay(source, prefix + 'iWrite c0.value 9; iApply;\n')
        self.reject(source, prefix + 'iWrite c1.value 6; iApply;\n')
        self.reject(source, prefix.replace('sel[0] 1', 'sel[0] 0') + 'iWrite c0.value 9; iApply;\n')
        self.replay(source, prefix + 'iRelease sel[0]; iWrite c1.value 6; iApply;\n')

    def test_take_does_not_widen_state_assumption_targets(self):
        for port in ('ck', 'shift[0]', 'si[0]'):
            with self.subTest(port=port):
                error = self.reject(helpers.network(), f'iTake {port}; iState {port} 0;\n')
                self.assertIn('state assumption', error)

    def test_invalid_reservation_target(self):
        source = helpers.network().replace('Module Chains {',
                                          'Module Chains { LogicSignal logic { 1\'b1; }')
        self.reject(source, 'iTake logic;\n')
        self.reject(helpers.network(), 'iTake missing;\n')

    def reject(self, source, body):
        icl, pdl = self.root / 'source.icl', self.root / 'source.pdl'
        icl.write_text(source)
        pdl.write_text(body)
        (self.root / 'result.pdl').write_bytes(b'previous output\n')
        error = self.run_tool(icl, pdl, ok=False)
        self.assertEqual((self.root / 'result.pdl').read_bytes(), b'previous output\n')
        return error


if __name__ == '__main__':
    unittest.main()
