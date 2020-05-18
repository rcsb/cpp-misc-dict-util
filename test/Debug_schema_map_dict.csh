#!/bin/csh -f
#
# File: Debug_schema_map_dict.csh
#       Build a schema map from the latest exchange dictionary
#       ...in debug mode... 
#
rm -f *.log 
#
# Add on some extra stuff defining the schema defintion for NDB ... 
#
cat ../../mmcif/mmcif_pdbx.dic sql-fragment.dic > tmp.dic

../bin/DictToSdb -ddlFile ../../mmcif/mmcif_ddl.dic \
  -dictFile tmp.dic -dictSdbFile tmp.sdb

echo "run -dictSdbFile tmp.sdb  -op standard -v -o new_schema.cif" > cmd.gdb
gdb -e ../bin/mk-schema-map-dict -x cmd.gdb
##

