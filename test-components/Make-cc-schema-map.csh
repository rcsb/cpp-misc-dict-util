#!/bin/csh -f
#
#
cat mmcif_pdbx_cc.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb

../bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard -o new_schema.cif

sed 's/Structure_ID/Component_ID/g' new_schema.cif | sed -f SedFixes.sed > schema_map_pdbx_cc.cif

