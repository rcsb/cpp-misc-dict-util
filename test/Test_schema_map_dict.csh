#!/bin/csh -f
#
# File: Test_schema_map_dict.csh
#       Build a schema map from the latest exchange dictionary, 
#       mmcif_pdbx_v40.dic including:
#
#       + All defined items + NDB extensions in sql-fragment.dic
#

#
# Add on some extra stuff defining the schema defintion for NDB ... 
#
cat ../../mmcif/mmcif_pdbx_v40.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb

#
../bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard \
  -o schema_map_pdbx_full.cif

#
../bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -f Item-list-template.cif \
  -o schema_map_pdbx_na.cif

##

