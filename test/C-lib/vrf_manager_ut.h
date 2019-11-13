/*
 Copyright (c) 2018-2019 AT&T Intellectual Property.
 All rights reserved.

 Copyright (c) 2016 by Brocade Communications Systems, Inc.
 All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * VRF manager UT helpers
 */

#ifndef __VRF_UT_H__
#define __VRF_UT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <linux/rtnetlink.h>
#include <sys/param.h>
#include "vrf_manager.h"
#include "vrf_manager_common.h"

#define ASSERT_IF_ID_NOT_SAME(id, output)				\
	if ((id) != (output))					\
	{							\
		printf("Unexpected Id %d received.", (output));	\
		printf(" Expected Id was %d\n", (id));		\
		printf("Test %s failed\n", __FUNCTION__);	\
		assert(0);					\
	}

#define ASSERT_IF_NAME_NOT_SAME(name, output) \
	if (strcmp(name, output) != 0)				\
	{							\
		printf("Unexpected name %s received,", output);	\
		printf(" Expected name was %s\n", name);	\
		printf("Test %s failed\n", __FUNCTION__);	\
		assert(0);					\
	}

#define TEST_START(name)					\
	void name()						\
	{

#define TEST_END						\
		printf("Test %s passed\n", __FUNCTION__);	\
	}

#endif
