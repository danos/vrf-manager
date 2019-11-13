#!/usr/bin/env python3
#
# Copyright (c) 2017-2019, AT&T Intellectual Property. All rights reserved.
# Copyright (c) 2016 by Brocade Communications Systems, Inc.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

"""Python3 vrf-manager library APIs."""

import os
import re
import subprocess
import os.path

vrfid_invalid = 0
vrfid_default = 1
vrfname_none = ""
vrfname_default = "default"
vrf_master_prefix = "vrf"

class VrfManagerException(BaseException):
    pass

class Vrf(object):
    """ Represents parameters of a single VRF """
    def __init__(self, rd_id, name, interface_num, interfaces_set=None):
        self._rd_id = int(rd_id)
        self._name = name
        self._interfaces_set = interfaces_set \
            if interfaces_set is not None else set()

    @property
    def id(self):
        return self._rd_id

    @property
    def name(self):
        return self._name

    @property
    def bound_interface_num(self):
        return len(self._interfaces_set)

    @property
    def interfaces(self):
        return self._interfaces_set

    @interfaces.setter
    def interfaces(self, interfaces_set):
        self._interfaces_set = interfaces_set

    def interface_is_bound(self, interface_s):
        """
        Returns True if the interface is bound to the VRF otherwise False
        """
        return interface_s in self.interfaces


class VrfManager(object):
    """ Vrf manager python 3 library class """

    @staticmethod
    def get_vrf_master_interface(vrf_name):
        return "{}{}".format(vrf_master_prefix, vrf_name)

    def __init__(self, filepath="/run/route-domain.conf"):
        # No longer used
        self._file = filepath

    def _get_lines_(self):
        """ Internal API to get the lines from getvrflist """
        output = ""
        try:
            output = subprocess.check_output([ "/usr/sbin/getvrflist" ],
                                             universal_newlines=True)
        except Exception as e:
            raise VrfManagerException("Failed to parse VRFs: {}".format(e))
        return output.split(os.linesep)

    def get_vrf_id(self, name):
        """ get_vrf_id will return the RD ID/VRF ID corresponding to
        the vrf name provided.
        Input: VRF name
        Output: RD ID on success, 0 on failure. """
        if name == vrfname_default:
            return vrfid_default
        if name == vrfname_none:
            return vrfid_invalid
        for line in self._get_lines_():
            token = line.split(' ')
            if len(token) < 2:
                continue
            if token[0] == name:
                return int(token[1].rstrip())
        else:
            return vrfid_invalid

    def get_vrf_name(self, rd_id):
        """ get_vrf_name will return the VRF name corresponding to
        the RD ID/VRF ID provided.
        Input: VRF ID
        Output: VRF name on success, empty string on failure. """
        if rd_id == vrfid_default:
            return vrfname_default
        if rd_id == vrfid_invalid:
            return vrfname_none
        for line in self._get_lines_():
            token = line.split(' ')
            if len(token) < 2:
                continue
            if int(token[1].rstrip()) == rd_id:
                return token[0]
        else:
            return vrfname_none

    def get_vrf_list(self):
        """ This member API will return all the mappings as yield of pair.
        Each pair is in a form '<vrf name>', <rd id>
        Input: None
        Output: Yield of "vrf name" and corresponding "rd id".
        Note: To use as a dictionary, use as "dict(get_vrf_list())"""
        for line in self._get_lines_():
            token = line.split(' ')
            if len(token) < 2:
                continue
            yield token[0].rstrip(), int(token[1].rstrip())

    def get_vrfs_dict(self):
        """
        Returns a dictionary of Vrf objects representing all running VRFs

        The dictionary is keyed by both name and id, ie. assuming a VRF with id 2
        and name "foo"; the key 2 and "foo" will return the same Vrf object

        Note that keys 2 and "2", ie. int(2) and str(2), MAY return
        different objects.
        """
        vrfs = {}
        for vrf in self.get_vrfs():
            if vrf.id in vrfs or vrf.name in vrfs:
                raise VrfManagerException("Duplicate VRF")
            vrfs[vrf.id] = vrf
            vrfs[vrf.name] = vrf
        return vrfs

    def get_vrfs(self):
        """
        Yields running VRFs represented by Vrf objects
        """
        output = ""

        vrfs = dict(self.get_vrf_list())
        for vrf in vrfs.keys():
            vrf = Vrf(vrfs[vrf], vrf, 0)

            try:
                output = subprocess.check_output([
                                "ip", "-br", "link", "list", "vrf",
                                VrfManager.get_vrf_master_interface(vrf.name)
                            ], universal_newlines=True)
            except Exception as e:
                raise VrfManagerException("Failed to parse VRFs: {}".format(e))
            for line in output.split(os.linesep):
                intf = line[:16].rstrip()
                if len(intf) == 0:
                    continue
                vrf.interfaces.add(intf)

            yield vrf
