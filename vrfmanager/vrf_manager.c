/*
 Copyright (c) 2018-2019 AT&T Intellectual Property.
 All rights reserved.

 Copyright (c) 2016 by Brocade Communications Systems, Inc.
 All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <sys/param.h>
#include <sys/socket.h>
#include "vrf_manager.h"
#include "vrf_manager_common.h"

#ifndef FILENAME
#define FILENAME "/run/route-domain.conf"
#endif

static int getwords(char *line, char *words[], int maxwords)
{
	char *p = line;
	int nwords = 0;

	while(1)
	{
		while(isspace(*p))
			p++;

		if(*p == '\0')
			return nwords;

		words[nwords++] = p;
		while(!isspace(*p) && *p != '\0')
			p++;

		if(*p == '\0')
			return nwords;
		*p++ = '\0';

		if(nwords >= maxwords)
			return nwords;
	}
}

static int getlinecount(FILE *fp)
{
	int count = 0;
	char c;

	for (c = getc(fp); c != EOF; c = getc(fp))
		if (c == '\n')
			count++;
	return count;
}

vrfid_t
get_vrf_id_upstream(const char* vrf_name)
{
	FILE *fp;
	char file[MAXPATHLEN];
	vrfid_t ret;

	if (!vrf_name)
		return VRFID_INVALID;

	if (strcmp(vrf_name, VRFNAME_DEFAULT) == 0)
		return VRFID_DEFAULT;

	snprintf(file, sizeof(file), "/sys/class/net/%s%s/ifindex",
		 VRFMASTER_PREFIX, vrf_name);
	fp = fopen(file , "r");
	if (!fp)
		return VRFID_INVALID;

	if (fscanf(fp, "%u", &ret) != 1)
		ret = VRFID_INVALID;

	fclose(fp);
	return ret;
}

vrfid_t
get_vrf_id(const char* vrf_name)
{
	return get_vrf_id_upstream(vrf_name);
}

bool
get_vrf_name_upstream(vrfid_t vrf_id, char **vrf_name)
{
	DIR *dir;
	struct dirent *de;
	bool ret = false;

	if (vrf_id == VRFID_INVALID)
		return false;

	if (vrf_id == VRFID_DEFAULT) {
		*vrf_name = strdup(VRFNAME_DEFAULT);
		if (!*vrf_name)
			return false;
		return true;
	}

	dir = opendir("/sys/class/net");
	if (!dir)
		return false;

	while ((de = readdir(dir)) != NULL) {
		if (strncmp(de->d_name, VRFMASTER_PREFIX,
			    VRFMASTER_PREFIX_LEN) == 0 &&
		    get_vrf_id_upstream(de->d_name +
					VRFMASTER_PREFIX_LEN) == vrf_id) {
			*vrf_name = malloc(VRF_NAME_SIZE);
			if (!*vrf_name)
				break;
			strncpy(*vrf_name, de->d_name + VRFMASTER_PREFIX_LEN,
				VRF_NAME_SIZE);
			(*vrf_name)[VRF_NAME_SIZE - 1] = '\0';
			ret = true;
			break;
		}
	}

	closedir(dir);
	return ret;
}

bool
get_vrf_name(vrfid_t vrf_id, char **vrf_name)
{
	return get_vrf_name_upstream(vrf_id, vrf_name);
}

unsigned int
get_vrf_list(struct vrf_map **list)
{
	FILE *fp = NULL;
	char *line = NULL;
	size_t len = 0;
	int nvrf = 0;
	int count = 0;

	fp = fopen(FILENAME , "r");
	if (!fp)
		return 0;

	nvrf = getlinecount(fp);
	rewind(fp);

	struct vrf_map *p = malloc(nvrf * sizeof(struct vrf_map));
	if (!p) {
		fclose(fp);
		return 0;
	}
	*list = p;

	/* The mappings might have been added or deleted by the time the list is
	 * filled. We are ignoring the newly added mappings as we have enough
	 * memory for only nvrf mappings */
	while (getline(&line, &len, fp) != -1 && count <= nvrf) {
		char *words[2] = {0};

		if (getwords(line, words, 2) == 2) {
			p->vrf_id = atol(words[1]);
			/* for now prefix "vrf", needed by snmp - VRVDR-40139 */
#if !defined(SO_RTDOMAIN)
			memcpy(p->vrf_name, "vrf", 3);
			strncpy(p->vrf_name + 3, words[0], sizeof(p->vrf_name) - 3);
#else
			strncpy(p->vrf_name, words[0], sizeof(p->vrf_name));
#endif
			p->vrf_name[VRF_NAME_SIZE - 1] = '\0';
			count++;
			p++;
		}

		FREE(line);
		line = NULL;
		len = 0;
	}

	/* free() the line as it has been allocated by getline() even if it
	 * fails */
	FREE(line);
	fclose(fp);

	return count;
}

tableid_t
get_vrf_kernel_table_id_upstream(const char* vrf_name, tableid_t pbr_tid)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	int ret = RT_TABLE_UNSPEC;

	if (!vrf_name)
		return ret;

	fp = fopen(FILENAME , "r");
	if (!fp)
		return ret;

	while (getline(&line, &len, fp) != -1)	{
		char *words[3] = {0};
		tableid_t tid;

		if (getwords(line, words, 3) == 3) {
			tid = atol(words[1]);
			if (tid == pbr_tid && strcmp(vrf_name, words[0]) == 0) {
				ret = atol(words[2]);
				break;
			}
		}

		FREE(line);
		len = 0;
	}

	/* free() the line as it has been allocated by getline() even if it
	 * fails */
	FREE(line);
	fclose(fp);
	return ret;
}

tableid_t
get_vrf_kernel_table_id(const char* vrf_name, tableid_t pbr_tid)
{
	return get_vrf_kernel_table_id_upstream(vrf_name, pbr_tid);
}
