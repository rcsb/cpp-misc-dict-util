#
#        MMCIF-MISC-DICT-UTIL module makefile
#
#----------------------------------------------------------------------------
# Project specific path defintions.
#----------------------------------------------------------------------------
M_INCL_DIR    = ../include
M_LIB_DIR     = ../lib
M_BIN_DIR     = ../bin

PROJ_DIR      = .

L_INCL_DIR    = $(PROJ_DIR)/include
SRC_DIR       = $(PROJ_DIR)/src
OBJ_DIR       = $(PROJ_DIR)/obj
L_LIB_DIR     = $(PROJ_DIR)/lib
L_BIN_DIR     = $(PROJ_DIR)/bin
TEST_DIR      = $(PROJ_DIR)/test

VPATH = $(OBJ_DIR) $(L_BIN_DIR)

#----------------------------------------------------------------------------
# LINCLUDES and LDEFINES are appended to CFLAGS and C++FLAGS
#----------------------------------------------------------------------------
LDEFINES  =
LINCLUDES = -I$(L_INCL_DIR) -I$(M_INCL_DIR)

#----------------------------------------------------------------------------
# Include the appropriate compiler/platform definitions ...
#----------------------------------------------------------------------------
include ../etc/Makefile.platform

# Dependent libraries
SCHEMA_MAP_LIB    = $(M_LIB_DIR)/schema-map.a
DICT_OBJ_FILE_LIB = $(M_LIB_DIR)/dict-obj-file.a
CIF_FILE_UTIL_LIB = $(M_LIB_DIR)/cif-file-util.a
CIF_FILE_LIB      = $(M_LIB_DIR)/cif-file.a
CIFPARSE_LIB      = $(M_LIB_DIR)/cifparse-obj.a
TABLES_LIB        = $(M_LIB_DIR)/tables.a
COMMON_LIB        = $(M_LIB_DIR)/common.a
REGEX_LIB         = $(M_LIB_DIR)/regex.a

ALL_DEP_LIBS = $(SCHEMA_MAP_LIB) $(DICT_OBJ_FILE_LIB) $(CIF_FILE_UTIL_LIB) \
  $(CIF_FILE_LIB) $(CIFPARSE_LIB) $(TABLES_LIB) $(COMMON_LIB) $(REGEX_LIB)

# Agregate library
AGR_LIB = all.a

# Temporary library. Used to obtain the agregate library.
TMP_LIB = tmp.a

L_AGR_LIB = $(L_LIB_DIR)/$(AGR_LIB)
M_AGR_LIB = $(M_LIB_DIR)/$(AGR_LIB)


# Base main file names. Must have ".ext" at the end of the file.
BASE_MAIN_FILES = cifexch.ext \
                  cifexch2.ext \
                  mk-schema-map-dict.ext \
                  CifCheck.ext \
                  cif_corrector.ext \
                  sf_corrector.ext \
                  DictInfo.ext \
                  DictToSdb.ext \
                  non_printable.ext

# Main source files. Replace ".ext" with ".C"
SRC_MAIN_FILES = ${BASE_MAIN_FILES:.ext=.C}

SRC_FILES = $(SRC_MAIN_FILES)

# Main object files. Replace ".ext" with ".o"
OBJ_MAIN_FILES = ${BASE_MAIN_FILES:.ext=.o}

ALL_OBJ_FILES = *.o

# Executables. Remove ".ext"
TARGETS = ${BASE_MAIN_FILES:.ext=}

# Scripts
TARGET_SCRIPTS = CreateDictSdbFile.csh

# Test related files
TEST_FILES = $(TEST_DIR)/Test.csh \
             $(TEST_DIR)/Test_schema_map_dict.csh \
             $(TEST_DIR)/Debug_schema_map_dict.csh \
             $(TEST_DIR)/sql-fragment.dic \
             $(TEST_DIR)/Item-list-template.cif \
             $(TEST_DIR)/sql-fragment.cif \
             $(TEST_DIR)/1aoa.cif \
             $(TEST_DIR)/rcsb010614.cif \
             $(TEST_DIR)/rcsb010917.cif \
             $(TEST_DIR)/rcsb012382.cif \
             $(TEST_DIR)/INFILELIST \
             $(TEST_DIR)/OUTFILELIST



.PHONY: ../etc/Makefile.platform all install test export clean clean_build clean_test

.PRECIOUS: $(OBJ_DIR)/%.o

# All
all: install


# Installation
install: $(TARGETS)


# Test
test: all
	@echo 'Running Test 1 ** Results stored in ./test/Test.log'
	@sh -c 'cd $(TEST_DIR); ./Test.csh > Test.log 2>&1'
	@sh -c 'cd $(TEST_DIR); ./Test_schema_map_dict.csh > Test_schema_map_dict.log 2>&1'
	@sh -c 'cd test-components; ./Make-cc-schema-map.csh > Make-cc-schema-map.log 2>&1'
	@sh -c 'cd test-internal-schema; ./Test_internal_schema_map_dict.csh > Test_internal_schema_map_dict.log 2>&1'
	@sh -c 'cd $(TEST_DIR); ./test-cifcheck.sh 2>&1 | tee test-cifcheck.log'
	@sh -c 'cd test-corrector; ./test-corrector.sh 2>&1 | tee test-corrector.log'


export:
	mkdir -p $(EXPORT_DIR)
	@cp Makefile $(EXPORT_DIR)/Makefile
	@cd $(EXPORT_DIR); mkdir -p $(L_INCL_DIR)
	@cd $(EXPORT_DIR); mkdir -p $(SRC_DIR)
	@cd $(SRC_DIR); $(EXPORT) $(EXPORT_LIST) $(SRC_FILES) ../$(EXPORT_DIR)/$(SRC_DIR)
	@cd $(EXPORT_DIR); mkdir -p $(OBJ_DIR)
	@cd $(EXPORT_DIR); mkdir -p $(L_BIN_DIR)
	@cd $(L_BIN_DIR); cp $(TARGET_SCRIPTS) ../$(EXPORT_DIR)/$(L_BIN_DIR)
	@cd $(EXPORT_DIR); mkdir -p $(TEST_DIR)
	@cp $(TEST_FILES) $(EXPORT_DIR)/$(TEST_DIR)


clean: clean_build clean_test


# Rule for making executables
%: $(OBJ_DIR)/%.o $(ALL_DEP_LIBS)
	$(CCC) $(LDFLAGS) $< $(ALL_DEP_LIBS) $(MALLOCLIB) -lm -o $(L_BIN_DIR)/$@
	@cp -f $(L_BIN_DIR)/$@ $(M_BIN_DIR)/$@
	@cp -f $(L_BIN_DIR)/$(TARGET_SCRIPTS) $(M_BIN_DIR)/$(TARGET_SCRIPTS)


# Rule for build cleaning
clean_build:
	@rm -f $(OBJ_DIR)/*.o
	@rm -rf $(OBJ_DIR)/ii_files
	@rm -f $(M_AGR_LIB)
	@cd $(L_BIN_DIR); rm -f $(TARGETS)
	@cd $(M_BIN_DIR); rm -f $(TARGETS) $(TARGET_SCRIPTS)


# Rule for test results cleaning
clean_test:
	@cd $(TEST_DIR); rm -f *.log *.tr *.tr.tr *.tr.diag *.sdb tmp.*
	@cd $(TEST_DIR); rm -f schema_map_pdbx_full.cif schema_map_pdbx_na.cif
	@cd test-components; rm -f *.log *.tr *.tr.tr *.tr.diag *.sdb tmp.*
	@cd test-components; rm -f new_schema.cif schema_map_pdbx_cc.cif
	@cd test-internal-schema; rm -f *.log *.tr *.tr.tr *.tr.diag *.sdb
	@cd test-internal-schema; rm -f schema_map_rcsb_internal.cif
	@cd test-internal-schema; rm -f tmp.* 
	@cd test-corrector; rm -f *.corrected *.log


# Rule for making object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.C
	$(CCC) $(C++FLAGS) -c $< -o $@


# Phony rule for making object files
%.o: $(SRC_DIR)/%.C
	$(CCC) $(C++FLAGS) -c $< -o $(OBJ_DIR)/$@

