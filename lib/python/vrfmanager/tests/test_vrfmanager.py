# Copyright (c) 2019 AT&T Intellectual Property. All rights reserved.
# Copyright (c) 2016 by Brocade Communications Systems, Inc.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import vrfmanager
import unittest
from unittest.mock import MagicMock, patch
import subprocess
from imp import reload

class TestVrf(unittest.TestCase):
        @classmethod
        def setUpClass(self):
            self.purple_vrf = vrfmanager.Vrf(2, "purple", 0)
            self.blue_vrf = vrfmanager.Vrf(3, "blue", 0, {"eth1", "eth2"})

        def test_id(self):
            self.assertEqual(self.purple_vrf.id, 2)
            self.assertEqual(self.blue_vrf.id, 3)

        def test_name(self):
            self.assertEqual(self.purple_vrf.name, "purple")
            self.assertEqual(self.blue_vrf.name, "blue")

        def test_bound_interface_num(self):
            self.assertEqual(self.purple_vrf.bound_interface_num, 0)
            self.assertEqual(self.blue_vrf.bound_interface_num, 2)

        def test_interfaces(self):
            self.assertEqual(self.purple_vrf.interfaces, set())
            self.assertEqual(self.blue_vrf.interfaces, {"eth1", "eth2"})

def side_effect_vrfs_subproc(args, universal_newlines=False):
        values = {
                '/usr/sbin/getvrflist' : """purple 2
blue 3""",
                'ip -br link list vrf vrfpurple' : """eth11            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>
eth12            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>
lord2            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>""",
                'ip -br link list vrf vrfblue' : """eth21            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>
lord3            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>""",
        }

        print(' '.join(args))
        return values[' '.join(args)]

def side_effect_vrfs_subproc2(args, universal_newlines=False):
        values = {
                '/usr/sbin/getvrflist' : """purple 2
blue 3""",
                'ip -br link list vrf vrfpurple' : """eth1             DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>
lord2            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>""",
                'ip -br link list vrf vrfblue' : """eth2             DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>
lord3            DORMANT        52:54:00:cf:33:ac <NO-CARRIER,BROADCAST,MULTICAST,UP,LOWER_UP>""",
        }

        print(' '.join(args))
        return values[' '.join(args)]

class TestVrfManager(unittest.TestCase):
        @classmethod
        def setUpClass(self):
            self.mapping = """RED 5
GREEN 10
PINK 222
55 12
RE 55
EEN 1000
"""
            self.VRF = vrfmanager.VrfManager()

        def tearDown(self):
            reload(subprocess)

        def test_valid_id(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.return_value = self.mapping

                self.assertEqual(10, self.VRF.get_vrf_id("GREEN"))
                self.assertEqual(12, self.VRF.get_vrf_id("55"))

                mock_subprocess.assert_called_with(
                    ["/usr/sbin/getvrflist"], universal_newlines=True)

        def test_valid_name(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.return_value = self.mapping

                self.assertEqual('RED', self.VRF.get_vrf_name(5))
                self.assertEqual('55', self.VRF.get_vrf_name(12))

                mock_subprocess.assert_called_with(
                    ["/usr/sbin/getvrflist"], universal_newlines=True)

        def test_list(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.return_value = self.mapping

                expected_d = {'RED': 5, '55': 12, 'RE': 55, 'GREEN': 10, 'PINK': 222, 'EEN': 1000}
                d = dict(self.VRF.get_vrf_list())
                self.assertEqual(d, expected_d)

                mock_subprocess.assert_called_with(
                    ["/usr/sbin/getvrflist"], universal_newlines=True)

        def test_default_name(self):
            self.assertEqual(vrfmanager.vrfname_default, self.VRF.get_vrf_name(self.VRF.get_vrf_id("default")))

        def test_default_id(self):
            self.assertEqual(vrfmanager.vrfid_default, self.VRF.get_vrf_id(self.VRF.get_vrf_name(1)));

        def test_invalid_name(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.return_value = self.mapping

                self.assertEqual(0, self.VRF.get_vrf_id("GREE"))
                self.assertEqual(0, self.VRF.get_vrf_id("GGREENN"))
                self.assertEqual(0, self.VRF.get_vrf_id(vrfmanager.vrfname_none))

                mock_subprocess.assert_called_with(
                    ["/usr/sbin/getvrflist"], universal_newlines=True)

        def test_invalid_id(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.return_value = self.mapping

                self.assertEqual("", self.VRF.get_vrf_name(1234))
                self.assertEqual("", self.VRF.get_vrf_name(2))
                self.assertEqual("", self.VRF.get_vrf_name(vrfmanager.vrfid_invalid))

                mock_subprocess.assert_called_with(
                    ["/usr/sbin/getvrflist"], universal_newlines=True)

        def test_get_vrfs_dict(self):
            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.side_effect = side_effect_vrfs_subproc

                vrfs = self.VRF.get_vrfs_dict()

            self.assertEqual(len(vrfs), 4)

            self.assertEqual(vrfs[2], vrfs["purple"])
            self.assertEqual(vrfs[2].id, 2)
            self.assertEqual(vrfs[2].name, "purple")
            self.assertEqual(vrfs[2].bound_interface_num, 3)
            self.assertEqual(vrfs[2].interfaces, {"eth11", "eth12", "lord2"})

            self.assertEqual(vrfs[3], vrfs["blue"])
            self.assertEqual(vrfs[3].id, 3)
            self.assertEqual(vrfs[3].name, "blue")
            self.assertEqual(vrfs[3].bound_interface_num, 2)
            self.assertEqual(vrfs[3].interfaces, {"eth21", "lord3"})

        def test_get_vrfs_valid_output(self):
            vrfs = {}

            with patch('subprocess.check_output') as mock_subprocess:
                mock_subprocess.side_effect = side_effect_vrfs_subproc2

                for vrf in self.VRF.get_vrfs():
                    vrfs[vrf.id] = vrf

            self.assertEqual(len(vrfs), 2)

            self.assertEqual(vrfs[2].id, 2)
            self.assertEqual(vrfs[2].name, "purple")
            self.assertEqual(vrfs[2].bound_interface_num, 2)
            self.assertEqual(vrfs[2].interfaces, {"eth1", "lord2"})

            self.assertEqual(vrfs[3].id, 3)
            self.assertEqual(vrfs[3].name, "blue")
            self.assertEqual(vrfs[3].bound_interface_num, 2)
            self.assertEqual(vrfs[3].interfaces, {"eth2", "lord3"})



if __name__ == "__main__":
    unittest.main()
