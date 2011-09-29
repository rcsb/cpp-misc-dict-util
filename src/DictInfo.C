#include <string.h>

#include <exception>
#include <string>
#include <iostream>

#include "ISTable.h"
#include "CifDataInfo.h"
#include "CifParentChild.h"
#include "DicFile.h"
#include "CifFileUtil.h"


using std::exception;
using std::string;
using std::cout;
using std::cerr;
using std::endl;


class CmdLineOpts
{
  public:
    string ddlFileName;
    string dictFileName;
    bool parChildExt;
    bool enumDisp;

    string progName;

    CmdLineOpts(unsigned int argc, char* argv[]);

  private:
    string _usage;

    void Usage();
};


static void ParentChildExtension(CifParentChild& cifParentChild,
  const string& dictFileName);
static void ExtractParentChild(Block& block, const string& dictFileName);
static void ExtractEnums(CifDataInfo& cifDataInfo);


int main(int argc, char** argv)
{
    try
    {
        CmdLineOpts opts(argc, argv);


        DicFile* ddlFileP = GetDictFile(NULL, opts.ddlFileName, string(),
          true);

        const string& ddlParsingDiags = ddlFileP->GetParsingDiags();

        if (!ddlParsingDiags.empty())
        {
            cout << "Dictionary file \"" << ddlFileP->GetSrcFileName() <<
              "\" parsing info = " << ddlParsingDiags << endl;
        }

        CheckDict(ddlFileP, ddlFileP, opts.ddlFileName);

        DicFile* dictFileP = GetDictFile(ddlFileP, opts.dictFileName);

        const string& dictParsingDiags = dictFileP->GetParsingDiags();

        if (!dictParsingDiags.empty())
        {
            cout << "Dictionary file \"" << dictFileP->GetSrcFileName() <<
              "\" parsing info = " << dictParsingDiags << endl;
        }

        CheckDict(dictFileP, ddlFileP, opts.dictFileName);

        delete (ddlFileP);


        Block& block = dictFileP->GetBlock(dictFileP->GetFirstBlockName());

        if (opts.parChildExt)
        {
            ExtractParentChild(block, opts.dictFileName);
        }

        if (opts.enumDisp)
        {
           CifDataInfo cifDataInfo(*dictFileP);
           ExtractEnums(cifDataInfo);
        }

        delete (dictFileP);
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
} /* End of main() */


static void ExtractParentChild(Block& block, const string& dictFileName)
{
    ISTable* itemLinkedTableP = block.GetTablePtr("item_linked");

    CifParentChild cifParentChild(block, itemLinkedTableP);

    ParentChildExtension(cifParentChild, dictFileName);
}


void ParentChildExtension(CifParentChild& cifParentChild,
  const string& dictFileName)
{
    // Prepare a CIF file for writing
    CifFile cifFile(false, Char::eCASE_SENSITIVE, 256);

    cifFile.AddBlock("parchild_extension");

    Block& block = cifFile.GetBlock("parchild_extension");

    (cifParentChild._groupTableP)->DeleteColumn("parent_category_id");
 
    block.WriteTable(cifParentChild._groupTableP);
    block.WriteTable(cifParentChild._groupListTableP);

    string extCifFileName = dictFileName + "-def-ilg.dic";

    cout << "Writing parent-child extension tables to file \"" <<
      extCifFileName << "\"" << endl;

    cifFile.Write(extCifFileName);

    cout << "Please remove the line that contians "\
      "\"data_parchild_extension\" before concatenation." << endl;
} /* End of ParentChildExtension() */


CmdLineOpts::CmdLineOpts(unsigned int argc, char* argv[]) : parChildExt(false),
  enumDisp(false)
{
    progName = argv[0];

    Usage();

    if (argc < 6)
    {
        throw InvalidOptionsException(_usage);
    }

    for (unsigned int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-ddl") == 0)
            {
                i++;
                ddlFileName = argv[i];
            }
            else if (strcmp(argv[i], "-dict") == 0)
            {
                i++;
                dictFileName = argv[i];
            }
            else if (strcmp(argv[i], "-pcExt") == 0)
            {
                parChildExt = true;
            }
            else if (strcmp(argv[i], "-enumDisp") == 0)
            {
                enumDisp = true;
            }
            else
            {
                throw InvalidOptionsException(_usage);
            }
        }
        else
        {
            throw InvalidOptionsException(_usage);
        }
    }

    if (dictFileName.empty() && ddlFileName.empty())
    {
        throw InvalidOptionsException(_usage);
    }
}


void CmdLineOpts::Usage()
{
    _usage = "\nUsage:\n\n" + progName +
      "\n  -dict <dictionary file> -ddl <DDL file> -pcExt | -enumDisp\n";
}


void ExtractEnums(CifDataInfo& cifDataInfo)
{
    const vector<string>& items = cifDataInfo.GetItemsNames();

    for (unsigned int itemI = 0; itemI < items.size(); ++itemI)
    {
        const string& item = items[itemI];

        vector<string> enums = cifDataInfo.GetItemAttribute(item,
          CifString::CIF_DDL_CATEGORY_ITEM_ENUMERATION,
          CifString::CIF_DDL_ITEM_VALUE);

        if (!enums.empty())
        {
            cout << "Item \"" << item << "\", has the following "\
              "enumerations: (";
            for (unsigned int enumI = 0; enumI < enums.size(); ++enumI)
            {
                cout << "\"" << enums[enumI] << "\"";
                if (enumI != enums.size() - 1)
                    cout << ", ";
            }
            cout << ")" << endl;
        }
    }
}

