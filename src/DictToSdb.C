#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <exception>
#include <iostream>
#include <string>

#include "RcsbFile.h"
#include "CifFile.h"
#include "DicFile.h"
#include "CifFileUtil.h"


using std::exception;
using std::string;
using std::cerr;
using std::cout;
using std::endl;


class CmdLineOpts
{
  public:
    string ddlFileName;
    string dictFileName;
    string dictSdbFileName;
    bool extraDictChecks;

    string progName;

    CmdLineOpts(unsigned int argc, char* argv[]);

    void Usage();
};


int main(int argc, char *argv[])
{
    try
    {
        // Parse command line arguments.
        CmdLineOpts opts(argc, argv);

        // CheckArgs();

        // CheckFileNames();
        // Check all arguments (existence of all files)

        cout << "Start of dictionary to .sdb conversion." << endl;
        cout << "Using DDL file: " << opts.ddlFileName << endl;
        cout << "Using dictionary file: " << opts.dictFileName << endl;
        cout << endl;
        cout << "Parsing logs are stored in files with \"-parser.log\" "\
          "extension. " << endl;
        cout << "Diagnostic logs are stored in files with \"-diag.log\" "\
          "extension. " << endl;
        cout << endl;

        DicFile* ddlFileP = GetDictFile(NULL, opts.ddlFileName, string(),
          false, UPDATE_MODE);

        const string& ddlParsingDiags = ddlFileP->GetParsingDiags();

        if (!ddlParsingDiags.empty())
        {
            cout << "Dictionary file \"" << ddlFileP->GetSrcFileName() <<
              "\" parsing info = " << ddlParsingDiags << endl;
        }

        cout << "Checking the DDL file against itself ..." << endl;

        CheckDict(ddlFileP, ddlFileP, opts.ddlFileName);

        DicFile* dictFileP = GetDictFile(ddlFileP, opts.dictFileName,
          string(), false, UPDATE_MODE);

        const string& dictParsingDiags = dictFileP->GetParsingDiags();

        if (!dictParsingDiags.empty())
        {
            cout << "Dictionary file \"" << dictFileP->GetSrcFileName() <<
              "\" parsing info = " << dictParsingDiags << endl;
        }

        cout << "Checking the dictionary file against the DDL ..." << endl;

        CheckDict(dictFileP, ddlFileP, opts.dictFileName, opts.extraDictChecks);

        string sdbFileName;
        if (opts.dictSdbFileName.empty())
        {
            RcsbFile::RelativeFileName(sdbFileName, opts.dictFileName);
            sdbFileName += ".sdb";
        }
        else
        {
            sdbFileName = opts.dictSdbFileName;
        } 

        dictFileP->Serialize(sdbFileName);

        delete (ddlFileP);

        delete (dictFileP);

        cout << endl;

        cout << "End of dictionary to .sdb conversion." << endl;
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
}


CmdLineOpts::CmdLineOpts(unsigned int argc, char* argv[])
{
    progName = argv[0];

    if (argc < 5)
    {
        Usage();
        throw InvalidOptionsException();
    }

    extraDictChecks = false;

    for (unsigned int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-ddlFile") == 0)
            {
                i++;
                ddlFileName = argv[i];
            }
            else if (strcmp(argv[i], "-dictFile") == 0)
            {
                i++;
                dictFileName = argv[i];
            }
            else if (strcmp(argv[i], "-dictSdbFile") == 0)
            {
                i++;
                dictSdbFileName = argv[i];
            }
            else if (strcmp(argv[i], "-ec") == 0)
            {
                extraDictChecks = true;
            }
            else
            {
                Usage();
                throw InvalidOptionsException();
            }
        }
        else
        {
            Usage();
            throw InvalidOptionsException();
        }
    }

    if (ddlFileName.empty() || dictFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }
}


void CmdLineOpts::Usage()
{
    cerr << endl << "Usage:" << endl << endl;
    cerr << progName << endl;
    cerr << "  -ddlFile <DDL ASCII file>" << endl;
    cerr << "  -dictFile <dictionary ASCII file> " << endl;
    cerr << "  [-dictSdbFile <dictionary SDB file>]" << endl;
    cerr << "  -ec  (extra checks, e.g., item type code)" << endl;
}

