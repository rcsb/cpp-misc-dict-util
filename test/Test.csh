#!/bin/csh -f

rm -f *.log *.tr *.tr.tr *.tr.diag

../bin/cifexch -dicSdb ../../sdb/mmcif_pdbx_v40.sdb \
               -inlist INFILELIST -op in -checkout -v

../bin/cifexch -dicSdb ../../sdb/mmcif_pdbx_v40.sdb \
               -inlist OUTFILELIST -op out -checkout -v

