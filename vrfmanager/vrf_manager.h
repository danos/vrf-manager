/*
 Copyright (c) 2018-2019 AT&T Intellectual Property.
 All rights reserved.

 Copyright (c) 2016 by Brocade Communications Systems, Inc.
 All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef __VRF_MGR_H__
#define __VRF_MGR_H__

#include <stdbool.h>
#include <stdint.h>

#define VRFID_INVALID	0
#define VRFID_DEFAULT	1
#define VRFID_MAX	4095
#define VRFNAME_DEFAULT "default"

#define VRFMASTER_PREFIX	"vrf"
#define VRFMASTER_PREFIX_LEN	(sizeof(VRFMASTER_PREFIX) - 1)

#define VRF_NAME_SIZE	65

typedef uint32_t vrfid_t;
typedef uint32_t tableid_t;

struct vrf_map {
	char vrf_name[VRF_NAME_SIZE];
	vrfid_t vrf_id;
};

/* API to get VRF ID from VRF name
 * get_vrf_id will get the VRF ID from the "vrf_name".
 * Return: If id is found, it will return the id, otherwise it will return
 * VRFID_INVALID. If "vrf_name" is NULL return VRFID_INVALID.
 */
vrfid_t
get_vrf_id(const char* vrf_name);

/* API to get VRF name from VRF ID
 * get_vrf_name will get the VRF name related to "vrf_id" provided. 
 * Memory for "vrf_name" will be allocated by the API using malloc() and MUST
 * be released by the caller. "vrf_name" buffer will be populated with the VRF
 * name which will be NULL terminated..
 * Return: If VRF name is found, "true" is returned. Otherwise "false" is returned.
 * If "vrf_name" pointer allocation fails, "false" is returned.
 * Note, this differs from the perl counterpart as in case of failure perl API
 * returns vrf name as VRFNAME_NONE.
 */
bool
get_vrf_name(vrfid_t vrf_id, char **vrf_name);

/* API to get list of VRF name to VRF ID mappings
 * This API will allocate the memory using malloc() and populate the "list"
 * with VRF mapping entries. User has to make sure to free ths memory using
 * free().
 * Return: Number of entries populated in "list". If "list" pointer allocation
 * fails, 0 is returned.
 */
unsigned int
get_vrf_list(struct vrf_map **list);

/* API to get kernel table ID from VRF name and PBR table id
 * Return: If id is found, it will return the id, otherwise it will return
 * RT_TABLE_UNSPEC.
 */
tableid_t
get_vrf_kernel_table_id(const char* vrf_name, tableid_t pbr_tid);

#endif
