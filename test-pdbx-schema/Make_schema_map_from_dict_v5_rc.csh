#!/bin/csh -f
#
# File: Make_schema_map_from_dict.csh
#       Build a schema map from the full pdbx dictionary
#
#       + All defined items + NDB extensions in sql-fragment.dic
#
#rm -f *.log
#
cat ../../mmcif/mmcif_pdbx_v5_rc.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb
#
../bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard \
  -o schema_map_pdbx_v5_rc.cif

#
##
