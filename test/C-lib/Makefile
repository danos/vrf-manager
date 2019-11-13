# Makefile for C library UT

UT_DIR = test/C-lib
CFG_DIR = test/testcfg
UT_SRC = $(UT_DIR)/vrf_manager_ut.c
LIB_SRC = vrfmanager/vrf_manager.c
INCLUDE_PATH = vrfmanager

# Change the conf file to point to predefined one in test directory
# FILENAME macro is passed with this updated value during compilation
CONF_FILE = "\"$(CFG_DIR)/route-domain.conf\""

# Test directory version of /sys/class/net
TEST_CLASS_NET = "\"$(CFG_DIR)/net\""

UT_EXE = vrf_manager_ut

.PHONY: test

test:
	$(CC) -g -D FILENAME=$(CONF_FILE) -D TEST_CLASS_NET=$(TEST_CLASS_NET) \
		-Wl,-wrap,fopen -Wl,-wrap,opendir \
		$(LIB_SRC) $(UT_SRC) -I$(INCLUDE_PATH) -o $(UT_DIR)/$(UT_EXE)
	./$(UT_DIR)/$(UT_EXE)

test_clean:
	rm -f $(UT_DIR)/$(UT_EXE)
