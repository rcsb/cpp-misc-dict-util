#!/bin/csh -f
#
# File: CreateDictSdbFile.csh
#

if ("$1" == "" ) then 
   echo "Usage:  $0 <Dictionary name (e.g. mmcif_pdbx)>"
   exit 1
endif

set d = "$1"

if (! -e dicts/dict-mmcif_ddl/mmcif_ddl.dic ) then 
   echo "Missing DDL file dicts/dict-mmcif_ddl/mmcif_ddl.dic"
   exit 1
endif

cd sdb 

set log = "$d.log"

rm -f ./$d.sdb

# If PDB exchange dictionary, do extended checks by specifying -ec flag
if ("$d" == "mmcif_pdbx_v40" ) then
    ../bin/DictToSdb -ddlFile ../dicts/dict-mmcif_ddl/mmcif_ddl.dic \
    -dictFile ../dicts/dict-$d/$d.dic -dictSdbFile $d.sdb >& $log
else
    ../bin/DictToSdb -ddlFile ../dicts/dict-mmcif_ddl/mmcif_ddl.dic \
    -dictFile ../dicts/dict-$d/$d.dic -dictSdbFile $d.sdb  >& $log
endif

chmod 644 $d.sdb

# These are used to create parent/child extensions tables from "item_linked"
#./bin/DictInfo -dict dict-$d/$d.dic -ddl dict-mmcif_ddl/mmcif_ddl.dic -pcExt
#sed "s/data_parchild_extension//g" < dict-$d/$d.dic-def-ilg.dic > dict-$d/tmp.txt
#mv dict-$d/tmp.txt dict-$d/$d-def-ilg.dic

if (-e mmcif_ddl.dic-parser.log) then
    echo " " >> $log
    echo " ========    DDL parser diagnostics ========= " >>  $log
    echo " " >> $log
    cat mmcif_ddl.dic-parser.log >> $log
    rm mmcif_ddl.dic-parser.log
else
    echo " " >> $log
    echo "No DDL parsing errors." >> $log
endif

if (-e mmcif_ddl.dic-diag.log) then
    echo " " >> $log
    echo " ========    DDL consistency diagnostics ========= " >>  $log
    echo " " >> $log
    cat mmcif_ddl.dic-diag.log >> $log
    rm mmcif_ddl.dic-diag.log
else
    echo " " >> $log
    echo "No DDL consistency errors." >> $log
endif

if (-e $d.dic-parser.log) then
    echo " " >> $log
    echo " ========    Dictionary parser diagnostics ========= " >>  $log
    echo " " >> $log
    cat $d.dic-parser.log >> $log
    rm $d.dic-parser.log
else
    echo " " >> $log
    echo "No dictionary parsing errors." >> $log
endif

if (-e $d.dic-diag.log) then
    echo " " >> $log
    echo " ========    Dictionary consistency diagnostics ========= " >>  $log
    echo " " >> $log
    cat $d.dic-diag.log >> $log
    rm $d.dic-diag.log
else
    echo " " >> $log
    echo "No dictionary consistency errors." >> $log
endif

cd -

