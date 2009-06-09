#!/bin/csh -f
#
# File: Test_schema_map_dict.csh
#       Build a schema map from the full internal dictionary
#
#       + All defined items + NDB extensions in sql-fragment.dic
#
#rm -f *.log 
#
# Add on some extra stuff defining the schema defintion for NDB ... 
#
cat ../../mmcif/mmcif_rcsb_internal.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb
#
../bin/mk-schema-map-dict -v -dictSdbFile tmp.sdb  -op standard \
  -o schema_map_rcsb_internal.cif

#
##
