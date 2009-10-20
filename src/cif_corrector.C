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
            DicFile* dictFileP = GetDictFile(NULL, string(),
              opts.dictSdbFileName);
           
            DicFile* pdbxDictFileP = GetDictFile(NULL, string(),
              opts.pdbxDictSdbFileName);

            CifDataInfo cifDataInfo(*dictFileP);

            CifDataInfo pdbxCifDataInfo(*pdbxDictFileP);

            CifFile* cifFileP = ParseCif(opts.inCifFileName);

            const string& parsingDiags = cifFileP->GetParsingDiags();

            if (!parsingDiags.empty())
            {
                cout << "Diags for file " << cifFileP->GetSrcFileName() <<
                  "  = " << parsingDiags << endl;
            }

            CifFile* configFileP = CifCorrector::CreateConfigFile();

            cout << "Begin: CIF corrector log for file \"" <<
              opts.inCifFileName << "\"" << endl;

            CifCorrector cifCorrector(*cifFileP, cifDataInfo, pdbxCifDataInfo,
              *configFileP, opts.verbose);

            cifCorrector.Correct();

            cifCorrector.CheckAliases();

            string outCifFileName;
            if (!opts.outCifFileName.empty())
            {
                outCifFileName = opts.outCifFileName;
            }
            else
            {
                CifCorrector::MakeOutputCifFileName(outCifFileName,
                  opts.inCifFileName);
            }

#ifdef VLAD_INTRODUCE_LATER
            CheckCif(cifFileP, dictFileP, outCifFileName);
#endif

            cifCorrector.Write(outCifFileName);

            cout << "End log: CIF corrector log for file \"" <<
              opts.inCifFileName << "\"" << endl;

            delete (dictFileP);
            delete (pdbxDictFileP);
            delete (configFileP);
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

    if (argc < 7)
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

    if (inCifFileName.empty() || dictSdbFileName.empty() ||
      pdbxDictSdbFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }
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

