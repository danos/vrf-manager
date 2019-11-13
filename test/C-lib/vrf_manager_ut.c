/*
 Copyright (c) 2018-2019 AT&T Intellectual Property.
 All rights reserved.

 Copyright (c) 2016 by Brocade Communications Systems, Inc.
 All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * VRF manager C library unit tests
 */
#include "vrf_manager_ut.h"

/* Which flavor of config file is being tested */
static const char *conf_file_suffix = "";

TEST_START(vrf_manager_ut_valid_id_upstream)
{
	/* Test if correct name is returned when a valid id is given. */
	char *name = NULL;
	vrfid_t id = 10;
	char *expected_name = "blue";

	assert(get_vrf_name_upstream(id, &name));
	ASSERT_IF_NAME_NOT_SAME(expected_name, name);
	FREE(name);

	id = 9;
	expected_name = "red";

	assert(get_vrf_name_upstream(id, &name));
	ASSERT_IF_NAME_NOT_SAME(expected_name, name);
	FREE(name);

	/* Test if default name is returned when queried with default id */
	id = VRFID_DEFAULT;
	expected_name = VRFNAME_DEFAULT;

	assert(get_vrf_name_upstream(id, &name));
	ASSERT_IF_NAME_NOT_SAME(expected_name, name);
	FREE(name);

	/* Test if default name is returned when queried with the id returned by
	 * get_vrf_id(VRFNAME_DEFAULT)
	 */
	expected_name = VRFNAME_DEFAULT;

	assert(get_vrf_name_upstream(get_vrf_id_upstream(VRFNAME_DEFAULT),
				     &name));
	ASSERT_IF_NAME_NOT_SAME(expected_name, name);
	FREE(name);
}
TEST_END

TEST_START(vrf_manager_ut_invalid_id_upstream)
{
	/* Test that get_vrf_name() returns false when queried with invalid id
	 */
	char *name = NULL;
	vrfid_t id = VRFID_INVALID;

	assert(get_vrf_name_upstream(id, &name) == false);

	/* Test that get_vrf_name() returns false when queried with the id not
	 * present in the config
	 */
	id = 3;

	assert(get_vrf_name_upstream(id, &name) == false);
}
TEST_END

TEST_START(vrf_manager_ut_valid_name_upstream)
{
	/* Test that a valid id is returned for a valid name present in the
	 * config. Proper string should be matched
	 */
	char *name = "blue";
	vrfid_t expected_id = 10;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));

	/* Test that valid id is returned for another valid name present in the
	 * config.
	 */
	name = "red";
	expected_id = 9;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));

	/* Test that valid id is returned for valid name which is a number in
	 * the config.
	 */
	name = "100";
	expected_id = 11;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));

	/* Test that VRFID_DEFAULT is returned when queried with name "default"
	 */
	name = VRFNAME_DEFAULT;
	expected_id = VRFID_DEFAULT;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));

	/* Test that VRFID_DEFAULT is returned when queried with name returned
	 * by get_vrf_name(VRFID_DEFAULT)
	 */
	name = NULL;
	expected_id = VRFID_DEFAULT;

	assert(get_vrf_name_upstream(VRFID_DEFAULT, &name));
	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));
	FREE(name);
}
TEST_END

TEST_START(vrf_manager_ut_invalid_name_upstream)
{
	/* Test if VRFID_INVALID is returned if the name is not found in the
	 * config
	 */
	char *name = "test3";
	vrfid_t expected_id = VRFID_INVALID;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));

	/* Test if VRFID_INVALID is returned if the name is NULL */
	name = NULL;
	expected_id = VRFID_INVALID;

	ASSERT_IF_ID_NOT_SAME(expected_id, get_vrf_id_upstream(name));
}
TEST_END

TEST_START(vrf_manager_ut_vrf_list)
{
	/* Test if all the mappings are returned on calling
	 * get_vrf_manager_ut_vrf_list(). Right now checking only the number of
	 * mappings received.
	 */
	struct vrf_map *list = NULL;
	uint32_t expected_count = 4;
	uint32_t count = get_vrf_list(&list);

	assert(count == expected_count);

	FREE(list);
}
TEST_END

TEST_START(vrf_manager_ut_kernel_table_id)
{
	tableid_t tid;

	/* Lookup valid table for valid vrf */
	tid = get_vrf_kernel_table_id_upstream("red", RT_TABLE_MAIN);
	ASSERT_IF_ID_NOT_SAME(257, tid);

	/* Lookup invalid table for valid vrf */
	tid = get_vrf_kernel_table_id_upstream("red", 1);
	ASSERT_IF_ID_NOT_SAME(tid, RT_TABLE_UNSPEC);

	/* Lookup table for invalid vrf */
	tid = get_vrf_kernel_table_id_upstream("test", RT_TABLE_MAIN);
	ASSERT_IF_ID_NOT_SAME(tid, RT_TABLE_UNSPEC);

	conf_file_suffix = "";
}
TEST_END

int main(int argc, char **argv)
{
	vrf_manager_ut_valid_id_upstream();
	vrf_manager_ut_invalid_id_upstream();
	vrf_manager_ut_valid_name_upstream();
	vrf_manager_ut_invalid_name_upstream();
	vrf_manager_ut_vrf_list();
	vrf_manager_ut_kernel_table_id();
	return 0;
}

/* Wrappers for system file manipulation */

#define SYS_CLASS_NET		"/sys/class/net"
#define SYS_CLASS_NET_LEN	(sizeof(SYS_CLASS_NET) -1)

static const char *convert_path_name (const char *in, char *out, size_t len)
{
	if (strcmp(in, FILENAME) == 0)
		/* open rd or upstream version of config file */
		snprintf(out, len, "%s%s", FILENAME, conf_file_suffix);
	else if (strncmp(in, SYS_CLASS_NET, SYS_CLASS_NET_LEN) == 0)
		/* open test directory version of /sys/class/net */
		snprintf(out, len, "%s%s", TEST_CLASS_NET,
			 in + SYS_CLASS_NET_LEN);
	else
		/* open file as is */
		return in;

	return out;
}

FILE * __real_fopen(const char *path, const char *mode);
FILE * __wrap_fopen(const char *path, const char *mode)
{
	char test_path[MAXPATHLEN];

	path = convert_path_name(path, test_path, sizeof(test_path));

	return __real_fopen(path, mode);
}

DIR * __real_opendir(const char *name);
DIR * __wrap_opendir(const char *name)
{
	char test_name[MAXPATHLEN];

	name = convert_path_name(name, test_name, sizeof(test_name));

	return __real_opendir(name);
}
