#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "RcsbFile.h"
#include "CifFile.h"
#include "DicFile.h"
#include "CifFileUtil.h"


using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::getline;


class CmdLineOpts
{
  public:
    string cifFileListName;
    string cifFileName;
    string dictSdbFileName;
    string dictFileName;
    string ddlFileName;
    bool extraCifChecks;
    string progName;

    CmdLineOpts(unsigned int argc, char* argv[]);

    void Usage();
};


static void GetFileNames(vector<string>& fileNames, const string& lFile);

static void localizeFile(const string& fileName, string& localFileName);

static bool isPrintable(const char ch);
static bool isCarriageReturn(char ch);
static void charToAsciiHexString(const char oneChar, string& asciiHexString);


int main(int argc, char *argv[])
{
    string str;

    unsigned int lineNo = 1;

    while (!std::cin.eof())
    {
        std::getline(std::cin, str);

        for (unsigned int charI = 0; charI < str.size(); ++charI)
        {
            if (isCarriageReturn(str[charI]))
            {
                cerr << "ERROR: Windows carriage return in line# " << lineNo \
                  << ", pos# " << charI + 1 << endl;

                continue;
            }

            if (!isPrintable(str[charI]))
            {
                string asciiHexString;
                charToAsciiHexString(str[charI], asciiHexString);
                cerr << "ERROR: Non-printable character in line# " << lineNo \
                  << ", pos# " << charI + 1 << ", hex code 0x" \
                  << asciiHexString << endl;
            }
        }

        ++lineNo;

        cout << "Line: " << str << endl;
    }

    return 0;



    try
    {
        // Parse command line arguments.
        CmdLineOpts opts(argc, argv);

        // CheckArgs();

        vector<string> fileNames;

        if (!opts.cifFileName.empty())
            fileNames.push_back(opts.cifFileName);

        if (!opts.cifFileListName.empty())
            GetFileNames(fileNames, opts.cifFileListName);

        // CheckFileNames();
        // Check all arguments (existence of all files)

        cout << "Start of CIF files checking." << endl;

        DicFile* dictFileP = NULL;

        if (opts.dictSdbFileName.empty())
        {
            cout << "Using DDL file: " << opts.ddlFileName << endl;
            cout << "Using dictionary file: " << opts.dictFileName << endl;
            cout << endl;

            DicFile* ddlFileP = GetDictFile(NULL, opts.ddlFileName);

            const string& ddlParsingDiags = ddlFileP->GetParsingDiags();
    
            if (!ddlParsingDiags.empty())
            {
                cout << "Dictionary file \"" << ddlFileP->GetSrcFileName() <<
                  "\" parsing info = " << ddlParsingDiags << endl;
            }

            cout << "Checking the DDL file against itself ..." << endl;

            CheckDict(ddlFileP, ddlFileP, opts.ddlFileName);

            dictFileP = GetDictFile(ddlFileP, opts.dictFileName);

            const string& dictParsingDiags = dictFileP->GetParsingDiags();
    
            if (!dictParsingDiags.empty())
            {
                cout << "Dictionary file \"" << dictFileP->GetSrcFileName() <<
                  "\" parsing info = " << dictParsingDiags << endl;
            }

            cout << "Checking the dictionary file against the DDL ..." << endl;

            CheckDict(dictFileP, ddlFileP, opts.dictFileName);

            delete (ddlFileP);

            cout << endl;
        }
        else
        {
            cout << "Using dictionary SDB file: " << opts.dictSdbFileName <<
              endl;

            dictFileP = GetDictFile(NULL, string(), opts.dictSdbFileName);

            cout << endl;
        }

        cout << "Parsing logs are stored in files with \"-parser.log\" "\
          "extension. " << endl;
        cout << "Diagnostic logs are stored in files with \"-diag.log\" "\
          "extension. " << endl;
        cout << endl;

        for (unsigned int fileI = 0; fileI < fileNames.size(); ++fileI)
        {
            // Make a local copy of the file if the filename contains a '/'
            string localFileName;

            localizeFile(fileNames[fileI], localFileName);

            CifFile* cifFileP = ParseCif(localFileName);

            const string& parsingDiags = cifFileP->GetParsingDiags();

            if (!parsingDiags.empty())
            {
                cout << "Diags for file " << cifFileP->GetSrcFileName() <<
                  "  = " << endl << parsingDiags << endl;
            }

            cout << "Checking the CIF file " << localFileName <<
              " against the dictionary ..." << endl;

            CheckCif(cifFileP, dictFileP, localFileName, opts.extraCifChecks);

            delete (cifFileP);

            //if (localFileName != fileNames[fileI])
            //  remove(localFileName.c_str());
        }

        delete (dictFileP);

        cout << "End of CIF files checking." << endl;
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
}


bool isPrintable(char ch)
{
    unsigned int asciiCode = ch;

    return ((asciiCode >= 48 && asciiCode <= 57) || \
      (asciiCode >= 97 && asciiCode <= 122) || \
      (asciiCode >= 65 && asciiCode <= 90) || \
      (asciiCode >= 33 && asciiCode <= 47) || \
      (asciiCode >= 58 && asciiCode <= 64) || \
      (asciiCode >= 91 && asciiCode <= 96) || \
      (asciiCode >= 123 && asciiCode <= 126) || \
      (asciiCode == 32) || \
      (asciiCode >= 9 && asciiCode <= 13));
}


bool isCarriageReturn(char ch)
{
    unsigned int asciiCode = ch;

    return asciiCode == 13;
}


void charToAsciiHexString(const char oneChar, string& asciiHexString)
{
    asciiHexString.clear();

    string maxAsciiStr = "FF";

    std::istringstream buffer(maxAsciiStr);
    unsigned int mask;
    buffer >> std::hex >> mask;

    unsigned int asciiCode = oneChar & mask;

    std::stringstream strStream;
    strStream << std::setw(maxAsciiStr.size()) << std::hex << asciiCode;

    asciiHexString = strStream.str();
}


void localizeFile(const string& fileName, string& localFileName)
{
  // if the file name contains path information make a local copy
  string::size_type idx;
  if (fileName.find("/") != string::npos) {
    idx=fileName.rfind("/");
    localFileName = fileName.substr(idx+1);
  } else {
    localFileName = fileName;
  }
  cout << "Input path " << fileName << endl;
  string cBuf;
  if (localFileName.substr(localFileName.size()-3) == ".gz") {
    localFileName = localFileName.substr(0,localFileName.size()-3);
    cBuf = "zcat " + fileName + " > " + localFileName;
  } else if (localFileName.substr(localFileName.size()-2) == ".Z")  {
    localFileName = localFileName.substr(0,localFileName.size()-3);
    cBuf = "zcat " + fileName + " > " + localFileName;
  } else if (localFileName != fileName)  {
    cBuf = "cp " + fileName + " " + localFileName;
  }
  cout << "Local file name " << localFileName << endl;
  if (cBuf.size() > 0) system(cBuf.c_str());
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


CmdLineOpts::CmdLineOpts(unsigned int argc, char* argv[])
{
    progName = argv[0];

    if (argc < 2)
    {
        Usage();
        throw InvalidOptionsException();
    }

    extraCifChecks = false;

    for (unsigned int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-l") == 0)
            {
                i++;
                cifFileListName = argv[i];
            }
            else if (strcmp(argv[i], "-f") == 0)
            {
                i++;
                cifFileName = argv[i];
            }
            else if (strcmp(argv[i], "-dictSdb") == 0)
            {
                i++;
                dictSdbFileName = argv[i];
            }
            else if (strcmp(argv[i], "-dict") == 0)
            {
                i++;
                dictFileName = argv[i];
            }
            else if (strcmp(argv[i], "-ddl") == 0)
            {
                i++;
                ddlFileName = argv[i];
            }
            else if (strcmp(argv[i], "-ec_pdbx") == 0)
            {
                extraCifChecks = true;
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

    if (cifFileListName.empty() && cifFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }

    if (!cifFileListName.empty() && !cifFileName.empty())
    {
        Usage();
        throw InvalidOptionsException();
    }

    if (!dictSdbFileName.empty())
    {
        if (!ddlFileName.empty() && !dictFileName.empty())
        {
            Usage();
            throw InvalidOptionsException();
        }
    }
    else
    {
        if (ddlFileName.empty() && dictFileName.empty())
        {
            Usage();
            throw InvalidOptionsException();
        }
    }
}


void CmdLineOpts::Usage()
{
    cerr << endl << "Usage:" << endl << endl;
    cerr << progName << endl;
    cerr << "  [-ec_pdbx] -f <CIF file> | -l <CIF files list>" << endl;
    cerr << "  -dictSdb <dictionary SDB file> | "
      << endl;
    cerr << "  -dict <dictionary ASCII file> -ddl <DDL ASCII file>" << endl;
}

