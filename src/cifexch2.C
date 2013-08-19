/*
 *  File:        cifexch.C
 */


#include <string.h>
#include <stdlib.h>

#include <exception>
#include <iostream>

#include "RcsbFile.h"
#include "CifString.h"
#include "CifFile.h"
#include "DicFile.h"
#include "CifFileUtil.h"


using std::exception;
using std::cout;
using std::cerr;
using std::getline;
using std::endl;
using std::ifstream;


struct Args
{
    string dicSdbFileName;
    string op;
    string inFileList;
    bool iCheckIn;
    bool iReorder;
    bool iRename;
    string idOpt;
    bool iCheckOut;
    bool iStrip;
    string citFile;
    string namFile;
    string srcFile;
    bool verbose;
};


static const char* skip_categories[] =
{
    "chem_comp_angle", "chem_comp_atom", "chem_comp_bond", "chem_comp_chir",
    "chem_comp_plane", "chem_comp_plane_atom", "chem_comp_tor", ""
};

static const char* category_name[] =
{
    "entry", "audit", "audit_conform", "database", "database_2",
    "database_PDB_rev", "database_PDB_rev_record",
    "pdbx_database_PDB_obs_spr", "pdbx_database_related",
    "pdbx_database_status", "pdbx_database_proc", "audit_contact_author",
    "audit_author", "citation", "citation_author", "citation_editor", "cell",
    "symmetry", "entity", "entity_keywords", "entity_name_com",
    "entity_name_sys", "entity_poly", "entity_poly_seq", "entity_src_gen",
    "entity_src_nat", "pdbx_entity_src_syn", "entity_link", "struct_ref",
    "struct_ref_seq", "struct_ref_seq_dif", "chem_comp", "pdbx_nmr_exptl",
    "pdbx_nmr_exptl_sample_conditions", "pdbx_nmr_sample_details",
    "pdbx_nmr_spectrometer", "pdbx_nmr_refine", "pdbx_nmr_details",
    "pdbx_nmr_ensemble", "pdbx_nmr_representative", "pdbx_nmr_software",
    "exptl", "exptl_crystal", "exptl_crystal_grow",
    "exptl_crystal_grow_comp", "diffrn", "diffrn_detector",
    "diffrn_radiation", "diffrn_radiation_wavelength", "diffrn_source",
    "reflns", "reflns_shell", "computing", "refine", "refine_analyze",
    "refine_hist", "refine_ls_restr", "refine_ls_restr_ncs",
    "refine_ls_shell", "pdbx_refine", "pdbx_xplor_file", "struct_ncs_oper",
    "struct_ncs_dom", "struct_ncs_dom_lim", "struct_ncs_ens",
    "struct_ncs_ens_gen", "struct", "struct_keywords", "struct_asym",
    "struct_biol", "struct_biol_gen", "struct_biol_view", "struct_conf",
    "struct_conf_type", "struct_conn", "struct_conn_type",
    "struct_mon_prot_cis", "struct_sheet", "struct_sheet_order",
    "struct_sheet_range", "struct_sheet_hbond", "pdbx_struct_sheet_hbond",
    "struct_site", "struct_site_gen", "database_PDB_matrix", "atom_sites",
    "atom_sites_alt", "atom_sites_footnote", "atom_type", "atom_site",
    "atom_site_anisotrop", "database_PDB_caveat", "database_PDB_remark",
    "pdbx_poly_seq_scheme", ""
};

static ISTable* items = NULL;
static ISTable* aliases = NULL;
static vector<string> QueryCol1;
static vector<string> QueryCol2;      
static string ColumnName;
static bool Verbose = false;
static DicFile* dictFileP = NULL;
static string dictVersion;

static void PrepareQueries(const string& op);
static void ProcessBlock(CifFile* fobjIn, const string& blockName,
  CifFile* fobjOut, const string& idOpt);
static bool SkipTable(const string& tableName);
static void ProcessTable(ISTable& inTable, Block& outBlock);
static void GetDataIntegrationFiles(CifFile*& fobjCit, CifFile*& fobjNam,
  CifFile*& fobjSrc, const string& citFile, const string& namFile,
  const string& srcFile);
static CifFile* ProcessInOut(const Args& args, CifFile& inCifFile);
static void add_stuff(CifFile* fobjIn, CifFile* fobjCit, CifFile* fobjNam,
  CifFile* fobjSrc);

static void ReplaceAttributeByEntity(CifFile *fobjIn, CifFile *fobData,
  const string& dataAttrib, const string& targetCategory,
  const string& targetAttribute, const string& bName,
  const string& entityID, const string& chainID);

static void update_entry_ids(CifFile* fobj, const string& blockId,
  const string& idName);
static void GetCatAndAttr(string& rCatName, string& rColName,
  const string& catName, const string& attribName);
static ISTable* GetOutTable(Block& outBlock, const string& catName);
static void ProcCatAndAttr(ISTable& outTable, const vector<string>& inCol,
  const string& rColName);
static void add_audit_conform(CifFile* fobj, const string& version);
static void StripFile(CifFile& outCifFile);
static void WriteOutFile(CifFile* fobjOut, const string& outFileCif,
  const bool iReorder);


static void usage(const string& pname)
{
    cerr << pname << ": " << endl 
      << "         usage = -dicSdb <filename>" << endl
      << "                 -op in|out -v (verbose)" << endl
      << "                 -inlist <filename>  " <<   endl
      << "                 -pdbids | -ndbids | -rcsbids " <<   endl
      << "                 -reorder  " <<   endl
      << "                 -checkin  " <<   endl
      << "                 -checkout  " <<   endl
      << "                 -rename  " <<   endl
      << "  Data integration data files: " << endl 
      << "                 -cit <cit_filename>  " <<   endl
      << "                 -nam <nam_filename>  " <<   endl;
}


static void GetArgs(Args& args, int argc, char* argv[])
{

    string pname = argv[0];

    if (argc < 3)
    {
        usage(pname);
        exit(1);
    }

    args.iCheckIn = false;
    args.iCheckOut = false;
    args.iReorder = false;
    args.iRename = false;
    args.iStrip = false;
    args.idOpt = "PDB";

    for (unsigned int i = 1; i < (unsigned int)argc; ++i)
    {
        string argVal = string(argv[i]);

        if (argVal[0] != '-')
        {
            usage(pname);
            exit(1);
        }

        if (argVal == "-dicSdb")
        {
            i++;
            argVal = string(argv[i]);
            args.dicSdbFileName = argVal;          
        }
        else if (argVal == "-op")
        {
            i++;
            argVal = string(argv[i]);
            args.op = argVal;
        }
        else if (argVal == "-inlist")
        {
            i++;
            argVal = string(argv[i]);
            args.inFileList = argVal;          
        }
        else if (argVal == "-checkin")
        {
            args.iCheckIn = true;
        }
        else if (argVal == "-reorder")
        {
            args.iReorder = true;
        }
        else if (argVal == "-rename")
        {
            args.iRename = true;
        }
        else if (argVal == "-pdbids")
        {
            args.idOpt = "PDB";
        }
        else if (argVal == "-ndbids")
        {
            args.idOpt = "NDB";
        }
        else if (argVal == "-rcsbids")
        {
            args.idOpt = "RCSB";
        }
        else if (argVal == "-checkout")
        {
            args.iCheckOut = true;
        }
        else if (argVal == "-strip")
        {
            args.iStrip = true;
        }
        else if (argVal == "-v")
        {
            args.verbose = true;
        }
        else if (argVal == "-cit")
        {
            i++;
            argVal = string(argv[i]);
            args.citFile = argVal;
        }
        else if (argVal == "-src")
        {
            i++;
            argVal = string(argv[i]);
            args.srcFile = argVal;
        }
        else if (argVal == "-nam")
        {
            i++;
            argVal = string(argv[i]);
            args.namFile = argVal;
        }
        else
        {
            usage(pname);
            exit(1);
        }
  }

  if (args.op.empty())
    exit(1);

  if ((args.op == "in") || (args.op == "out"))
  {
    if (args.dicSdbFileName.empty())
    {
      usage(pname);
      exit(1);
    }
  }

}


static void GetFileNames(vector<string>& fileNames, const string& lFile)
{

    fileNames.clear();

    ifstream infile(lFile.c_str());

    while (true)
    {
        string fileName;
        getline(infile, fileName);

        if (infile.eof() || infile.fail())
            break;

        fileNames.push_back(fileName);
    }

    infile.close();

}

// CifTranslator
// CifTranslator::CifTranslator(dictSdbFileName)
// items, aliases, dictVersion, dictFileP

// CifTranslator::
int main(int argc, char* argv[])
{
    try
    {
        Args args;
        GetArgs(args, argc, argv);

        Verbose = args.verbose;

        dictFileP = GetDictFile(NULL, string(), args.dicSdbFileName, Verbose);

        Block& block = dictFileP->GetBlock(dictFileP->GetFirstBlockName());

        items = block.GetTablePtr("item");
        aliases = block.GetTablePtr("item_aliases");

        if ((items == NULL) || (aliases == NULL))
        {
            cerr << "Dictionary tables for items and aliases are missing." <<
              endl;
        }

        if (Verbose)
        {
            cerr << " ALIAS TABLE " << endl << endl;
            cout << (*aliases);
            cerr << " ITEMS TABLE " << endl << endl;
            cout << (*items);
        }

        items->SetFlags("name", ISTable::DT_STRING | ISTable::CASE_INSENSE);

        aliases->SetFlags("name", ISTable::DT_STRING | ISTable::CASE_INSENSE);
        aliases->SetFlags("alias_name", ISTable::DT_STRING |
          ISTable::CASE_INSENSE);

        ISTable* dictionaryP = block.GetTablePtr("dictionary");
        dictVersion = (*dictionaryP)(0, "version");

        PrepareQueries(args.op);

        vector<string> fileNames;
        GetFileNames(fileNames, args.inFileList);

        for (unsigned int i = 0; i < fileNames.size(); ++i)
        {
            const string& inCifFileName = fileNames[i];

            if (Verbose)
            {
                cerr << "INFO - Translating file " << inCifFileName << endl;
            }

            CifFile* fobjIn = ParseCif(inCifFileName, Verbose);

            const string& parsingDiags = fobjIn->GetParsingDiags();

            if (!parsingDiags.empty())
            {
                cout << "Diags for file " << fobjIn->GetSrcFileName() <<
                  "  = " << parsingDiags << endl;
            }

            // VLAD - Shouldn't this go above the ParseCif line
            if (Verbose)
            {
                cerr << "INFO - Read file " << inCifFileName << endl;
            }

            if (args.iCheckIn)
            {
                if (Verbose)
                {
                    cerr << "INFO - Checking file " << inCifFileName << endl;
                }
                string diagFile = inCifFileName + ".diag";
                fobjIn->DataChecking(*dictFileP, diagFile);
            }

            CifFile* fobjOut = ProcessInOut(args, *fobjIn);

            string idCode;

            const string& blockName = fobjOut->GetFirstBlockName();

            fobjIn->GetAttributeValueIf(idCode, blockName, "database_2",
              "database_code", "database_id", args.idOpt);
            if (idCode.empty())
            {
                cerr << "ERROR - Cannot get " << args.idOpt <<
                  " id code for " << blockName << endl;
                continue;
            }

            String::UpperCase(idCode);

            if (Verbose)
            {
                cerr << "INFO - idCode is " << idCode << " block name " <<
                  blockName << endl;
            }

            update_entry_ids(fobjOut, fobjOut->GetFirstBlockName(), idCode);

            string outFileCif;

            if (args.iRename && !idCode.empty())
            {
                // Rename output file following idcode semantics.
                outFileCif = idCode;
                String::LowerCase(outFileCif);
                outFileCif += ".cif";

                // Check if the output and input file are the same
                string relInFileName;
                RcsbFile::RelativeFileName(relInFileName, inCifFileName);

                if (relInFileName == outFileCif)
                {
                    cerr << "ERROR - Cannot rename file \"" << relInFileName <<
                      "\" to \"" << outFileCif << "\". File \"" <<
                      inCifFileName << "\" not processed." << endl;

                    delete (fobjOut);
                    delete (fobjIn);

                    continue;
                }
            }
            else
            {
                outFileCif = inCifFileName + ".tr";
            }

            if (args.iCheckOut)
            {
                if (Verbose)
                {
                    cerr << "INFO - Checking file " << outFileCif << endl;
                }
                string diagFile = outFileCif + ".diag";
                fobjOut->DataChecking(*dictFileP, diagFile);
            }

            fobjOut->RenameFirstBlock(idCode);

            WriteOutFile(fobjOut, outFileCif, args.iReorder);

            delete (fobjOut);
            delete (fobjIn);
        }

        delete (dictFileP);

        if (Verbose)
        {
            cerr  << "INFO - Done " << endl;
        }
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
}


CifFile* ProcessInOut(const Args& args, CifFile& inCifFile)
{
    // Data integration data files  
    CifFile* fobjCit = NULL;
    CifFile* fobjNam = NULL;
    CifFile* fobjSrc = NULL;

    if (!args.citFile.empty() && !args.namFile.empty() &&
      !args.srcFile.empty())
    {
        GetDataIntegrationFiles(fobjCit, fobjNam, fobjSrc, args.citFile,
          args.namFile, args.srcFile);
    }

    CifFile* fobjOut = new CifFile(Verbose, Char::eCASE_SENSITIVE, 132);

    vector<string> blockNamesIn;
    inCifFile.GetBlockNames(blockNamesIn);

    if (Verbose)
    {
        cerr << "INFO - nBlocks =  " << blockNamesIn.size() << endl;
        cerr << "INFO -   idOpt =  " << args.idOpt << endl;
    }

    for (unsigned int ib = 0; ib < blockNamesIn.size(); ++ib)
    {
        if (Verbose)
        {
            cerr << "INFO - Block  " << ib << " of " <<
              blockNamesIn.size() << " is " << blockNamesIn[ib] << endl;
        }

        ProcessBlock(&inCifFile, blockNamesIn[ib], fobjOut, args.idOpt);
    }

    if ((fobjCit != NULL) && (fobjNam != NULL) && (fobjSrc != NULL))
    {
        add_stuff(fobjOut, fobjCit, fobjNam, fobjSrc);
    }

    add_audit_conform(fobjOut, dictVersion);

    if (args.iStrip)
    {
        StripFile(*fobjOut);
    }

    return (fobjOut);
}


void PrepareQueries(const string& op)
{
    if (op == "in")
    {
        ColumnName = "name";
        QueryCol1.push_back("alias_name");
    }
    else
    {
        ColumnName = "alias_name";
        QueryCol1.push_back("name");
    }

    QueryCol1.push_back("dictionary");
    QueryCol1.push_back("version");

    QueryCol2.push_back("name");
}


void ProcessBlock(CifFile* fobjIn, const string& blockName, CifFile* fobjOut,
  const string& idOpt)
{
    Block& inBlock = fobjIn->GetBlock(blockName);

    if (Verbose)
    {
        cerr << "INFO - ids updated " << endl;
        cerr  << "INFO - bName is " << blockName  << endl;
    }

    fobjOut->AddBlock(blockName);     
    Block& outBlock = fobjOut->GetBlock(blockName);
    if (blockName.empty())
    {
        // VLAD - shouldn't this be at the beginning????
        return;
    }

    vector<string> tableList;
    inBlock.GetTableNames(tableList);

    for (unsigned int it=0; it < tableList.size(); it++)
    {
        ISTable* inTableP = inBlock.GetTablePtr(tableList[it]);

        if (!SkipTable(inTableP->GetName()))
        {
            ProcessTable(*inTableP, outBlock);
        }
        else
        {
            cerr << "INFO - Skipping table \"" << inTableP->GetName() <<
              "\"" << endl;
        }
    }
}


bool SkipTable(const string& tableName)
{
    for (unsigned int i = 0; true; ++i)
    {
        if (strlen(skip_categories[i]) > 0)
        {
            if (string(skip_categories[i]) == tableName)
            {
                return (true);
            }
        }
        else
        {
            break;
        }
    }

    return (false);
}


void ProcessTable(ISTable& inTable, Block& outBlock)
{
    if (Verbose)
    {
        cerr << "INFO - Table " << inTable.GetName() << endl;
    }

    const vector<string>& colNames = inTable.GetColumnNames();

    for (unsigned int colInd = 0; colInd < colNames.size(); ++colInd)
    {
        string rColName;
        string rCatName;

        GetCatAndAttr(rCatName, rColName, inTable.GetName(), colNames[colInd]);

        if (rCatName.empty())
        {
            if (Verbose)
            {
                string cs;
                CifString::MakeCifItem(cs, inTable.GetName(), colNames[colInd]);
                String::LowerCase(cs);

                cerr << "WARNING - Translator skipping item "<< cs 
                  << ".  Item is NOT in dictionary." << endl;
            }

            continue;
        }

        ISTable* outTableP = GetOutTable(outBlock, rCatName);

        vector<string> inCol;
        inTable.GetColumn(inCol, colNames[colInd]);

        if ((outTableP->GetNumRows() != 0) && (outTableP->GetNumRows() !=
          inCol.size()))
        {
            cerr << "ERROR - Different number of rows in category \"" <<
              inTable.GetName() << "\" and in category \"" << rCatName <<
              "\"" << endl;
            continue;
        }

        ProcCatAndAttr(*outTableP, inCol, rColName);

        outBlock.WriteTable(outTableP);
    }
}


void GetDataIntegrationFiles(CifFile*& fobjCit, CifFile*& fobjNam,
  CifFile*& fobjSrc, const string& citFile, const string& namFile,
  const string& srcFile)
{
    fobjCit = ParseCif(citFile, Verbose, Char::eCASE_SENSITIVE, 132);

    fobjNam = ParseCif(namFile, Verbose, Char::eCASE_SENSITIVE, 132);

    fobjSrc = ParseCif(srcFile, Verbose, Char::eCASE_SENSITIVE, 512);
}


void GetCatAndAttr(string& rCatName, string& rColName, const string& catName,
  const string& attribName)
{
    string cs;
    CifString::MakeCifItem(cs, catName, attribName);
    String::LowerCase(cs);

    vector<string> queryTarget1;      

    queryTarget1.push_back(cs);
    queryTarget1.push_back("cif_rcsb.dic");
    queryTarget1.push_back("1.1"); 

    unsigned int queryResult1 = aliases->FindFirst(queryTarget1, QueryCol1);

    if (queryResult1 == aliases->GetNumRows())
    {
        //  No alias name  ...
        vector<string> queryTarget2;      
        queryTarget2.push_back(cs);

        unsigned int queryResult2 = items->FindFirst(queryTarget2, QueryCol2);
        if (queryResult2 == items->GetNumRows())
        {
            return;
        }
        else
        {
            const string& itemName = (*items)(queryResult2, QueryCol2[0]);
            CifString::GetItemFromCifItem(rColName, itemName);
            CifString::GetCategoryFromCifItem(rCatName, itemName);
        }
    }
    else
    {
        const string& aliasName = (*aliases)(queryResult1, ColumnName);
        CifString::GetItemFromCifItem(rColName, aliasName);
        CifString::GetCategoryFromCifItem(rCatName, aliasName);
    }
}


ISTable* GetOutTable(Block& outBlock, const string& catName)
{
    if (outBlock.IsTablePresent(catName))
    {
        return (outBlock.GetTablePtr(catName));
    }
    else
    {
        // Create new table for alias column.
        return (new ISTable(catName));
    }
}


void ProcCatAndAttr(ISTable& outTable, const vector<string>& inCol,
  const string& rColName)
{
    if (outTable.IsColumnPresent(rColName))
    {
        // Overwrite any existing column ...
        string rCifItem;
        CifString::MakeCifItem(rCifItem, outTable.GetName(), rColName);
        cerr << "WARNING - Alias " << rCifItem <<
          " maps to existing column" << endl;

        outTable.FillColumn(rColName, inCol);
    }
    else
    {
        // Copy column to existing table 
        outTable.AddColumn(rColName, inCol);
    }
}


void StripFile(CifFile& fobjOut)
{
    vector<string> outBlocksNames;
    fobjOut.GetBlockNames(outBlocksNames);

    for (unsigned int ib = 0; ib < outBlocksNames.size(); ++ib)
    {
        const string& bName = outBlocksNames[ib];
        Block& outBlock = fobjOut.GetBlock(bName);
        if (bName.empty())
        {
            continue;
        }

        vector<string> tableList;
        outBlock.GetTableNames(tableList);
        for (unsigned int it = 0; it < tableList.size(); ++it)
        {
            if (String::IsCiEqual(tableList[it], "entry"))
                continue;

            ISTable* t = outBlock.GetTablePtr(tableList[it]);
            unsigned int nRows = t->GetNumRows();
            if (nRows != 1)
            {
                continue;
            }

            // Table has one row

            unsigned int nBlank = 0;

            const vector<string>& aRow = t->GetRow(0);

            for (unsigned int ic = 0; ic < aRow.size(); ++ic)
            {
                if (CifString::IsEmptyValue(aRow[ic]))
                {
                    // missing
                    nBlank++;
                }
            }

            const vector<string>& colNames = t->GetColumnNames();
            if (nBlank == colNames.size() || (String::IsCiEqual(colNames[0],
              "entry_id") && (nBlank == colNames.size() - 1)))
            {
                t->DeleteRow(0);
                outBlock.WriteTable(t);
                if (Verbose)
                    cerr << "INFO - Stripping table " << tableList[it] << endl;
            }
        } // for (all tables in the block)
    } // for (all blocks in the file)
}


void WriteOutFile(CifFile* fobjOut, const string& outFileCif,
  const bool iReorder)
{

    if (iReorder)
    {
        if (Verbose)
        {
            cerr << "INFO - Reordering file " << outFileCif << endl;
        }

        vector<string> catList;
        for (unsigned int i = 0; true; ++i)
        {
            if (strlen(category_name[i]) > 0)
            {
                catList.push_back(category_name[i]);
            }
            else
                break;
        }

        fobjOut->Write(outFileCif, catList);
    }
    else
    {
        fobjOut->Write(outFileCif);
    }

}


void add_audit_conform(CifFile* fobj, const string& version)
{

    ISTable* tN = new ISTable("audit_conform");

    tN->AddColumn("dict_name");
    tN->AddColumn("dict_version");
    tN->AddColumn("dict_location");

    vector<string> row;

    row.push_back("mmcif_pdbx.dic");
    row.push_back(version);
    row.push_back("http://mmcif.pdb.org/dictionaries/ascii/mmcif_pdbx.dic");

    tN->AddRow(row);

    Block& block = fobj->GetBlock(fobj->GetFirstBlockName());

    block.WriteTable(tN);

}


void add_stuff(CifFile* fobjIn, CifFile* fobjCit, CifFile* fobjNam,
  CifFile* fobjSrc)
{

    vector<string> values;
    string cs1;
    string cs2;
    string cs3;
    int irow;

    string csX;
    vector<string> names;
    vector<string> types;
    ISTable *tN;

    // Start here.
    ISTable *t1 = NULL;
    string srcType;
    string val;

    cerr << "INFO - Starting integration step" << endl;

    string bName = fobjIn->GetFirstBlockName();

    cerr << "INFO - Block = " << bName << endl;

    string idCode;
    fobjIn->GetAttributeValueIf(idCode, bName, "database_2",
      "database_code", "database_id", "PDB");
    if (idCode.empty())
    {
        cout << "Cannot get PDB id code for input file."<< endl;
      return;
    }
    cerr << "INFO - ID " <<  idCode << endl;


    fobjIn->GetAttributeValueIf(val, bName, "citation", "journal_abbrev",
      "id", "primary");
    if (String::IsCiEqual(val, "To be Published"))
    {
        cerr << "INFO - Attempting replacement of missing citation" << endl;
        // 
        // Rename existing primary citation to primary-prev
        //
#if 1
        fobjIn->SetAttributeValueIf(bName, "citation", "id", "prev-primary",
          "id", "primary");
    
        fobjIn->SetAttributeValueIf(bName, "citation_author", "citation_id",
          "prev-primary", "citation_id", "primary");

        fobjIn->SetAttributeValueIf(bName, "citation_editor", "citation_id",
          "prev-primary", "citation_id", "primary");
#else
        // This is broken.. 
        del_attribute_value_where(fobjIn, bName.c_str(), "citation", "id",
          "primary");
    
        del_attribute_value_where(fobjIn, bName.c_str(), "citation_author",
          "citation_id", "primary");

        del_attribute_value_where(fobjIn, bName.c_str(), "citation_editor",
          "citation_id", "primary");
#endif
    
        val.clear();
        Block& inBlock = fobjIn->GetBlock(bName);
        t1=inBlock.GetTablePtr("citation");
        if (t1 != NULL)
        {
            const vector<string>& colNames = t1->GetColumnNames();
            unsigned int nCols = t1->GetNumColumns();
            if ((!colNames.empty()) && (nCols > 0))
            {
                for (unsigned int i=0; i < nCols; i++)
                { 
                    fobjCit->GetAttributeValueIf(val, idCode, "citation",
                      colNames[i], "id", "primary");
                    if (!val.empty())
                    {
                        fobjIn->SetAttributeValueIf(bName, "citation", 
                          colNames[i], val, "id", "primary", true);

                        val.clear();
                    }
               }
           }
        }

        t1=inBlock.GetTablePtr("citation_author");
        if (t1 != NULL)
        { 
            fobjCit->GetAttributeValuesIf(values, idCode, "citation_author",
              "name", "citation_id", "primary");
            if (!values.empty())
            {
                cs1.clear(); cs1 = "primary";
                if (t1->IsColumnPresent("citation_id") &&
                  t1->IsColumnPresent("name"))
                {
                    for (unsigned int i=0; i < values.size(); i++)
                    {
                        if (values[i].size() > 1)
                        {  
                            cs2.clear(); cs2 = values[i];
                            t1->AddRow();
                            irow = t1->GetNumRows() - 1;
                            t1->UpdateCell(irow, "citation_id", cs1);
                            t1->UpdateCell(irow, "name", cs2);
                        }
                    }
                }
                values.clear();
            }
        }
    }
    else if (!val.empty())
    {
      cerr << "INFO - Using existing citation: " << val << endl;    
      val.clear();
    }
  
    fobjIn->GetAttributeValuesIf(values, bName, "entity", "id", "type", "polymer");

    if (values.empty())
    {
        fobjIn->GetAttributeValuesIf(values, bName, "entity", "id",
          "type", "POLYMER");
    }

    cerr << "INFO - Polymer entities =  " << values.size()  << endl;    
    if (!values.empty())
    {
        for (unsigned int i=0; i < values.size(); i++)
        {
            if (!CifString::IsEmptyValue(values[i]))
            {
                string chId;
                fobjIn->GetAttributeValueIf(chId, bName,
                  "pdbx_poly_seq_scheme", "pdb_strand_id",
                  "entity_id", values[i]);

                if (!chId.empty())
                {
                    if (chId[0] == '?'  || chId[0] == '.')
                        chId[0] = '_'; 

                    csX.clear(); csX = idCode; csX += ":"; csX += chId;
                    cerr << "INFO - Entity " << values[i] << " maps to  chain " << chId << " " << csX << endl; 
                    fobjNam->GetAttributeValuesIf(names, "namedata", "rcsb_data", "mm_name",
                      "pdb_and_chain_id", csX);
                    fobjNam->GetAttributeValuesIf(types, "namedata", "rcsb_data", "name_type",
                      "pdb_and_chain_id", csX);
                    if (!names.empty() && !types.empty())
                    {
                        cerr << "Found  " <<  names.size() << " names." << endl;
                        tN = new ISTable("pdbx_entity_name");
                        tN->AddColumn("entity_id");
                        tN->AddColumn("name");
                        tN->AddColumn("name_type");
                        for (unsigned int j=0; j < names.size(); j++)
                        {
                            if (names[j].size() > 1)
                            {
                               cerr << "Name: " << names[j] << " type " << types[j] << endl;
		               tN->AddRow();
		               irow = tN->GetNumRows() - 1;
		               cs1.clear(); cs1 = values[i];
		               cs2.clear(); cs2 = names[j];
		               cs3.clear(); cs3 = types[j];
		               tN->UpdateCell(irow, "entity_id", cs1);
		               tN->UpdateCell(irow, "name", cs2);
		               tN->UpdateCell(irow, "name_type", cs3);
	                    }
	                    names[j].clear();
	                    types[j].clear();
	                }

                        Block& inBlock = fobjIn->GetBlock(fobjIn->GetFirstBlockName());
                        inBlock.WriteTable(tN);		      
	                names.clear();
	                types.clear();
	            }
	            // Source items here...

                    //loop_
	            //_rcsb_data.pdb_id 
	            //_rcsb_data.pdb_chain_id 
	            //_rcsb_data.seq_id 
	            //_rcsb_data.mm_name 
	            //_rcsb_data.mm_source_organism 
	            //_rcsb_data.mm_expression_system 
	            //_rcsb_data.mm_source_type 
	            //_rcsb_data.mm_source_organ 
	            //_rcsb_data.mm_source_plasmid 
	            //_rcsb_data.mm_source_cell_line 
	            //_rcsb_data.mm_source_gene 
	            //_rcsb_data.mm_expression_system_strain 
	            //_rcsb_data.mm_expression_system_vector_type 
	            //_rcsb_data.mm_expression_system_vector_gene 
	            //_rcsb_data.mm_tissue 
	            //_rcsb_data.name_type 
	            //_rcsb_data.pdb_and_chain_id 



	            fobjSrc->GetAttributeValueIf(srcType, "nistdata", "rcsb_data", "mm_source_type",
					   "pdb_and_chain_id", csX);

	            if (String::IsCiEqual(srcType, "engineered"))
                    {
	                cerr << "INFO - Source type =  " << srcType  << endl;    
	                ReplaceAttributeByEntity(fobjIn, fobjSrc, "mm_source_organism",
				     "entity_src_gen", "pdbx_gene_src_scientific_name",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_expression_system",
				     "entity_src_gen", "pdbx_host_org_scientific_name",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_expression_system_vector_type",
				     "entity_src_gen", "pdbx_host_org_vector_type",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_expression_system_strain",
				     "entity_src_gen", "pdbx_host_org_strain",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_plasmid",
				     "entity_src_gen", "plasmid_name",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_gene",
				     "entity_src_gen", "pdx_gene_src_gene",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_cell_line",
				     "entity_src_gen", "pdx_gene_src_cell_line",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_organ",
				     "entity_src_gen", "pdx_gene_src_organ",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_expression_system_vector_gene",
				     "entity_src_gen", "pdx_host_org_gene",
				     bName, values[i], csX);

	            }
                    else if (String::IsCiEqual(srcType, "natural"))
                    {

	                cerr << "INFO - Source type =  " << srcType  << endl;    
	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_organism",
				     "entity_src_nat", "pdbx_organism_scientific",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn,fobjSrc, "mm_source_organ",
				     "entity_src_nat", "pdbx_organ",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn, fobjSrc, "mm_source_cell_line",
				     "entity_src_nat", "pdbx_cell_line",
				     bName, values[i], csX);
 
  	                ReplaceAttributeByEntity(fobjIn, fobjSrc, "mm_source_cell_line",
				     "entity_src_nat", "pdbx_cell_line",
				     bName, values[i], csX);

	                ReplaceAttributeByEntity(fobjIn, fobjSrc, "mm_source_tissue",
				     "entity_src_nat", "tissue",
				     bName, values[i], csX);
	    

	            }
                    else if (String::IsCiEqual(srcType, "synthetic"))
                    {
	                cerr << "INFO - Source type =  " << srcType  << endl;
	            }
	        }
            }
      
        }
        values.clear();
    }

    cerr << "INFO - Completed integration step for " << bName << endl;
}


static void ReplaceAttributeByEntity(CifFile *fobjIn, CifFile *fobjData,
  const string& dataAttribute, const string& targetCategory,
  const string& targetAttribute, const string& bName, const string& entityID,
  const string& PDBAndchainID) 
{
    string dataValue;

    fobjData->GetAttributeValueIf(dataValue, "nistdata", "rcsb_data",
      dataAttribute, "pdb_and_chain_id", PDBAndchainID);

    if (!dataValue.empty() && dataValue[0] != '.' && dataValue[0] == '?')
    {
        string targetValue; 
        fobjIn->GetAttributeValueIf(targetValue, bName, targetCategory,
          targetAttribute, "entity_id", entityID);
        if (targetValue.empty() || targetValue[0] == '.' ||
          targetValue[0] == '?')
        {
            fobjIn->SetAttributeValueIf(bName, targetCategory, 
              targetAttribute, dataValue, "entity_id", entityID, true);

            string targetCifItem;
            CifString::MakeCifItem(targetCifItem, targetCategory,
              targetAttribute);

            cerr << "INFO - "<< targetCifItem << " in " << 
              bName << " entity " << entityID <<  " set to " << 
              dataValue << endl; 
        }
        else
        {
            if (!String::IsCiEqual(dataValue, targetValue))
            {
                string targetCifItem;
                CifString::MakeCifItem(targetCifItem, targetCategory,
                  targetAttribute);
                cerr << "WARNING - "<< targetCifItem << " differ in " << 
                  bName << " entity " << entityID <<  " " << 
                  targetValue << " .ne. " << dataValue << endl; 
            }
        }
    } 

}


void update_entry_ids(CifFile* fobj, const string& blockId, const string& id)
{
    string idName;
    String::UpperCase(id, idName);

    fobj->SetAttributeValue(blockId, "entry", "id", idName, true);    

    fobj->SetAttributeValue(blockId, "atom_sites", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "cell", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "cell_measurement", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "chemical", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "chemical_formula", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "computing", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "database", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "database_PDB_matrix", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ebi_soln_scatter", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_2d_projection_selection", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "em_3d_fitting", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_3d_reconstruction", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "em_assembly", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_detector", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_electron_diffraction", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "em_electron_diffraction_pattern",
      "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_electron_diffraction_phase",
      "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_euler_angle_distribution", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "em_image_scans", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_imaging", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "em_sample_preparation", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "em_vitrification", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "entry_link", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "exptl", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "geom", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "journal", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_atlas", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_coord", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_database_PDB_master", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_database_message", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_database_pdb_omit", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_database_proc", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_database_status", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_database_status",
      "replaced_entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_na_struct_keywds", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_refine", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_rms_devs_covalent", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_struct_conf_na", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "ndb_struct_feature_na", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "ndb_summary_flags", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "pdbx_helical_symmetry", "entry_id",
     idName);
    fobj->SetAttributeValue(blockId, "pdbx_phasing_MR", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "pdbx_phasing_dm", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "pdbx_point_symmetry", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "pdbx_re_refinement", "entry_id", idName);

    fobj->SetAttributeValue(blockId, "pdbx_database_proc", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_database_status", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_refine", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_constraints", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_details", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_ensemble", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_ensemble_rms", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_force_constants", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_refine", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "pdbx_nmr_representative", "entry_id",
      idName);

    fobj->SetAttributeValue(blockId, "phasing_MAD", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "phasing_MIR", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "phasing_averaging", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "phasing_isomorphous", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "publ", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "publ_manuscript_incl", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_data_processing_cell", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_data_processing_detector",
      "entry_id", idName);
    fobj->SetAttributeValue(blockId, "rcsb_data_processing_reflns", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_computing", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_constraints", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_details", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_ensemble", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_ensemble_rms", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_force_constants", "entry_id",
      idName);    
    fobj->SetAttributeValue(blockId, "rcsb_nmr_refine", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "rcsb_nmr_representative", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_post_process_details", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "rcsb_post_process_status", "entry_id",
      idName);
    fobj->SetAttributeValue(blockId, "refine", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "refine_analyze", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "reflns", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "struct", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "struct_keywords", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "struct_mon_details", "entry_id", idName);
    fobj->SetAttributeValue(blockId, "symmetry", "entry_id", idName);

    // Make db codes consistent
    string ndbID;
    fobj->GetAttributeValueIf(ndbID, blockId, "database_2", "database_code",
      "database_id", "NDB");
    if (!ndbID.empty())
    {
        fobj->SetAttributeValue(blockId, "database", "ndb_code_NDB", ndbID);
    }

    string pdbID;
    fobj->GetAttributeValueIf(pdbID, blockId, "database_2", "database_code",
      "database_id", "PDB");
    if (!pdbID.empty())
    {
      fobj->SetAttributeValue(blockId, "database", "ndb_code_PDB", pdbID);
    }

}

