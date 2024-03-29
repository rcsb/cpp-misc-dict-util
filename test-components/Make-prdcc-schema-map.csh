#!/bin/csh -f
#
#
set builddir=../../../build

cat mmcif_pdbx_prdcc.dic sql-fragment.dic > tmp.dic

$builddir/bin/DictToSdb -ddlFile $builddir/mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb

$builddir/bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard -o new_schema.cif

sed 's/Structure_ID/Component_ID/g' new_schema.cif | sed -f SedFixes.sed > schema_map_pdbx_prdcc.cif

