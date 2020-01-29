# CHEM_COMP
s/\(chem_comp.*mon_nstd_parent_comp_id.*0 0\) 10 \(.*\)/\1 50 \2/
s/\(chem_comp.*mon_nstd_parent .*0 0\) 10 \(.*\)/\1 20 \2/
s/\(chem_comp.*name .*0 0\) 80 \(.*\)/\1 1024 \2/
s/\(chem_comp.*one_letter_code.*0 0\) 2 \(.*\)/\1 5 \2/
s/\(chem_comp.*three_letter_code.*0 0\) 4 \(.*\)/\1 5 \2/
s/\(chem_comp.*pdbx_synonyms.*0 0\) 80 \(.*\)/\1 512 \2/
s/\(chem_comp.*pdbx_model_coordinates_db_code.*0 0\) 80 \(.*\)/\1 30 \2/
s/\(chem_comp.*pdbx_subcomponent_list.*0 0\) 200 \(.*\)/\1 80 \2/
s/\(chem_comp.*pdbx_release_status.*0 0\) 80 \(.*\)/\1 8  \2/
s/\(chem_comp.*pdbx_stnd_atom_id.*0 0\) 80 \(.*\)/\1 6  \2/
## PDBX_CHEM_COMP_DESCRIPTOR
s/\(pdbx_chem_comp_descriptor.*descriptor.*0 0\) 200 \(.*\)/\1 2048 \2/
#
# PDBX_CHEM_COMP_IDENTIFIER
s/\(pdbx_chem_comp_identifier.*identifier.*0 0\) 200 \(.*\)/\1 2048 \2/
#
# PDBX_CHEM_COMP_AUDIT
s/\(pdbx_chem_comp_audit.*action_type.*1 1\) 10 \(.*\)/\1 80 \2/
# PDBX_CHEM_COMP_SYNONYMS
s/\(pdbx_chem_comp_synonyms.*name .*0 0\) 200 \(.*\)/\1 1024 \2/
