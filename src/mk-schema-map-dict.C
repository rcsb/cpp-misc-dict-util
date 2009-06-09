/*
 *  File:     mk-schema-map-dict.C
 *  Date:     09-Apr-2002  J. Westbrook
 *
 * Build schema mapping file from a dictionary.  Optionally apply forward and reverse aliasing.
 *
 * Updated: 22-Feb-2006 J. Westbrook  - Tested with current dictionaries.
 *                                      Reorganized name CIF->SQL name munging.
 *                                      Fixed all outstanding bugs.
 *
 */

#include <string.h>
#include <exception>
#include <iostream>

#include "SchemaMap.h"

using std::exception;
using std::cout;
using std::cerr;
using std::endl;

class CmdLineOpts
{
  public:
    string dictSdbFileName;
    string op;
    bool verbose;
    string inFile;
    string structId;
    string outFile;

    string progName;

    CmdLineOpts(int argc, char* argv[]);

    void Usage();
};


int main(int argc, char* argv[])
{

    try
    {
        CmdLineOpts opts(argc, argv);

        SchemaMap schemaMap(CREATE_MODE, opts.dictSdbFileName, opts.verbose);

        cout << "Done reading dictionaries" << endl;

        schemaMap.Create(opts.op, opts.inFile, opts.structId);

        schemaMap.Write(opts.outFile);

        cout << "Done " << endl;

        return(0);
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return(1);    
    }

}


CmdLineOpts::CmdLineOpts(int argc, char* argv[]) : verbose(false)
{

    progName = argv[0];

    if (argc < 2)
    {
        Usage();
        throw InvalidOptionsException();
    }

    for (unsigned int i = 1; i < (unsigned int)argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-dictSdbFile") == 0)
            {
                i++;
                dictSdbFileName = argv[i];
            }
            else if (strcmp(argv[i], "-op") == 0)
            {
                i++;
                op = argv[i];
            }
            else if (strcmp(argv[i], "-v") == 0)
            {
                 verbose = true;
            }
            else if (strcmp(argv[i], "-f") == 0)
            {
                i++;
                inFile = argv[i];
            }
            else if (strcmp(argv[i], "-struct_id") == 0)
            {
                i++;
                structId = argv[i];
            }
            else if (strcmp(argv[i], "-o") == 0)
            {
                i++;
                outFile = argv[i];
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

    if (dictSdbFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }

    if (!structId.empty())
    {
       if ((structId != SchemaMap::_DATABLOCK_ID) &&
         (structId != SchemaMap::_DATABASE_2) &&
         (structId != SchemaMap::_ENTRY_ID))
       {
           Usage();
           throw InvalidOptionsException();
       }
    }
    else
    {
        if (inFile.empty())
        {
            structId = SchemaMap::_DATABLOCK_ID;
        }
        else
        {
            structId = SchemaMap::_DATABASE_2;
        }
    }

}

void CmdLineOpts::Usage()
{
    cerr << progName << ": " << endl <<
      "                 -dictSdbFile <dictionary SDB filename>" << endl <<
      "                 -op  alias|unalias|standard  -v (verbose)" << endl <<
      "                 -f <template mmCIF filename>" << endl <<
      "                 -struct_id <entry_id | datablock_id | database_2>" <<
      endl <<
      "                 -o <output schema mapping file name>" << endl;
}

