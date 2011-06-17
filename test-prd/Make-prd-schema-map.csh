#!/bin/csh -f
#
#
cat mmcif_pdbx-header.dic  prd-extension-v4.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb

../bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard -o new_schema.cif

sed 's/Structure_ID/prd_entry_id/g' new_schema.cif > schema_map_pdbx_prd.cif

