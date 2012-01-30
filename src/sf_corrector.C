//$$FILE$$
//$$VERSION$$
//$$DATE$$
//$$LICENSE$$


#include <exception>
#include <string>
#include <iostream>

#include "Exceptions.h"
#include "DictObjCont.h"
#include "DictObjFile.h"
#include "DictDataInfo.h"
#include "CifDataInfo.h"
#include "CifFileUtil.h"
#include "CifCorrector.h"


using std::exception;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

void sfCorrect(CifFile& cifFile);

class CmdLineOpts
{
  public:
    string inCifFileName;
    string dictSdbFileName;
    string pdbxDictSdbFileName;
    string outCifFileName;
    bool verbose;

    string progName;

    CmdLineOpts(int argc, char* argv[]);

    void Usage();
};


int main(int argc, char* argv[])
{
    try
    {
        CmdLineOpts opts(argc, argv);

        {
            //DicFile* dictFileP = GetDictFile(NULL, string(),
            //  opts.dictSdbFileName);
           
            //DicFile* pdbxDictFileP = GetDictFile(NULL, string(),
            //  opts.pdbxDictSdbFileName);

            //CifDataInfo cifDataInfo(*dictFileP);

            //CifDataInfo pdbxCifDataInfo(*pdbxDictFileP);


            CifFile* cifFileP = ParseCif(opts.inCifFileName);

            const string& parsingDiags = cifFileP->GetParsingDiags();

            if (!parsingDiags.empty())
            {
                cout << "Diags for file " << cifFileP->GetSrcFileName() <<
                  "  = " << parsingDiags << endl;
            }

            // CifFile* configFileP = CifCorrector::CreateConfigFile();

            cout << "Begin: SF corrector log for file \"" <<
              opts.inCifFileName << "\"" << endl;

            // Read bad SF file

            // Correc it
            sfCorrect(*cifFileP);
       
            // Write the file under different name


            //CifCorrector cifCorrector(*cifFileP, cifDataInfo, pdbxCifDataInfo,
            //  *configFileP, opts.verbose);

            //cifCorrector.Correct();

            //cifCorrector.CheckAliases();

            string outCifFileName = opts.inCifFileName + string(".sfcorr");

            //if (!opts.outCifFileName.empty())
            //{
            //    outCifFileName = opts.outCifFileName;
            //}
            //else
            //{
            //    CifCorrector::MakeOutputCifFileName(outCifFileName,
            //      opts.inCifFileName);
            //}

#ifdef VLAD_INTRODUCE_LATER
            CheckCif(cifFileP, dictFileP, outCifFileName);
#endif

            //cifCorrector.Write(outCifFileName);
            cifFileP->Write(outCifFileName);

            cout << "End log: SF corrector log for file \"" <<
              opts.inCifFileName << "\"" << endl;

            //delete (dictFileP);
            //delete (pdbxDictFileP);
            //delete (configFileP);
            delete (cifFileP);
        }
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
}


CmdLineOpts::CmdLineOpts(int argc, char* argv[])
{
    progName = argv[0];

    //if (argc < 7)
    if (argc < 3)
    {
        Usage();
        throw InvalidOptionsException();
    }

    verbose = false;

    for (unsigned int i = 1; i < static_cast<unsigned int>(argc); ++i)
    {
        if (argv[i][0] == '-')
        {
            if (string(argv[i]) == "-inCifFile")
            {
                i++;
                inCifFileName = argv[i];          
            }
            else if (string(argv[i]) == "-dictSdbFile")
            {
                i++;
                dictSdbFileName = argv[i];
            }
            else if (string(argv[i]) == "-pdbxDictSdbFile")
            {
                i++;
                pdbxDictSdbFileName = argv[i];
            }
            else if (string(argv[i]) == "-outCifFile")
            {
                i++;
                outCifFileName = argv[i];
            }
            else if (string(argv[i]) == "-v")
            {
	        verbose = true;
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

    /*
    if (inCifFileName.empty() || dictSdbFileName.empty() ||
      pdbxDictSdbFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }
    */
}


void CmdLineOpts::Usage()
{
    cerr << progName << " usage =  " << endl;
    cerr << "  -inCifFile <CIF file name>" << endl;
    cerr << "  -dictSdbFile <internal dictionary SDB file>" << endl;
    cerr << "  -pdbxDictSdbFile <pdbx dictionary SDB file>" << endl;
    cerr << "  [-outCifFile <output CIF file name>]" << endl;
    cerr << "  [-v (verbose option)]"  << endl;
}


void sfCorrect(CifFile& cifFile)
{
    string reflnsCatName = "reflns";
    string databaseTwoCatName = "database_2";
    string auditCatName = "audit";
    string revisionIdAttr = "revision_id";
    string creationDateAttr = "creation_date";

    vector<string> blockNames;
    cifFile.GetBlockNames(blockNames);
    for (unsigned int blockInd = 0; blockInd < blockNames.size(); ++blockInd)
    {
        const string& blockName = blockNames[blockInd];
        Block& block = cifFile.GetBlock(blockName);

        // Delete "reflns" table, if exists
        if (block.IsTablePresent(reflnsCatName))
        {
            block.DeleteTable(reflnsCatName);
        }

        // Delete "database_2" table, if exists
        if (block.IsTablePresent(databaseTwoCatName))
        {
            block.DeleteTable(databaseTwoCatName);
        }

        // Fix "_audit.revision_id" item, if "audit" cateogry exists
        if (!block.IsTablePresent(auditCatName))
        {
            cerr << "ERROR: No audit category in block \"" << blockName <<
               "\"" << endl;
        }
        else
        {
            ISTable& auditTable = block.GetTable(auditCatName);
            if (!auditTable.IsColumnPresent(revisionIdAttr))
            {
                vector<string> revisionIdValues;
                for (unsigned int rowInd = 0; rowInd < auditTable.GetNumRows();
                    ++rowInd)
                {
                    revisionIdValues.push_back(string("1_") + 
                      String::IntToString(rowInd));
                }

                auditTable.InsertColumn(revisionIdAttr, creationDateAttr,
                  revisionIdValues);
                cout << "INFO: Inserted _audit.revision_id item in \"" <<
                  blockName << "\"" << endl;
            }
        } // "audit" category is present
    } // For every block in the file
} // End of sfCorrect()

