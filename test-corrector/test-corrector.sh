#!/bin/sh
#
../bin/cif_corrector -inCifFile 2jjl.cif.cif.badcase.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee 2jjl.cif.cif.badcase.cif.log

../bin/cif_corrector -inCifFile rcsb050022.cif.cif.sequence.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050022.cif.cif.sequence.cif.log

../bin/cif_corrector -inCifFile rcsb050195.cif.cif.corrvalues.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050195.cif.cif.corrvalues.cif.log

../bin/cif_corrector -inCifFile rcsb050195.cif.cif.enums.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050195.cif.cif.enums.cif.log

../bin/cif_corrector -inCifFile rcsb050195.cif.cif.missing.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050195.cif.cif.missing.cif.log

../bin/cif_corrector -inCifFile rcsb050195.cif.cif.notappl.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050195.cif.cif.notappl.cif.log

../bin/cif_corrector -inCifFile rcsb050195.cif.cif.numlist.cif -dictSdbFile ../../sdb/mmcif_rcsb_internal.sdb -pdbxDictSdbFile ../../sdb/mmcif_pdbx.sdb -v 2>&1 | tee rcsb050195.cif.cif.numlist.cif.log
