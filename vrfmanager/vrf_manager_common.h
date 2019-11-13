/*
 Copyright (c) 2018-2019, AT&T Intellectual Property. All rights reserved.

 Copyright (c) 2016 by Brocade Communications Systems, Inc.
 All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause
*/

/* Common definitions for source code and UT */
#ifndef __VRF_MGR_COMMON_H__
#define __VRF_MGR_COMMON_H__

#define FREE(x)                 \
        __extension__           \
        ({                      \
                  free(x);      \
                  x = NULL;     \
          })

vrfid_t get_vrf_id_upstream(const char* vrf_name);
bool get_vrf_name_upstream(vrfid_t vrf_id, char **vrf_name);
tableid_t get_vrf_kernel_table_id_upstream(const char* vrf_name, tableid_t tid);

#endif
