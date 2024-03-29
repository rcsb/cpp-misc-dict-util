#!/bin/csh -f
#
#
set echo
#
set builddir=../../../build
#
echo "Step 1 ---------------"
cat mmcif_pdbx-header.dic mmcif_pdbx-data.dic  prd-extension-v6.dic mmcif_pdbx_prdcc.dic sql-fragment.dic > tmp.dic
echo $status
ls -la
#
echo "Step 2 ---------------"
$builddir/bin/DictToSdb -ddlFile $builddir/mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb
echo $status
ls -la
#
echo "Step 3 ---------------"
$builddir/bin/mk-schema-map-dict -dictSdbFile tmp.sdb  -op standard -o new_schema_v5.cif
echo $status
ls -la
#
echo "Step 4 ---------------"
sed 's/Structure_ID/db_id/g' new_schema_v5.cif > schema_map_pdbx_prd_v5.cif
echo $status
ls -la
#
