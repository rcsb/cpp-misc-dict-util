#include <iomanip>
#include <istream>
#include <ostream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

#include "Exceptions.h"
#include "GenString.h"


using std::exception;
using std::istream;
using std::ostream;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::string;
using std::getline;


static void non_printable(std::istream& inStream, std::ostream& outStream,
  bool strip);


class CmdLineOpts
{
  public:
    string progName;

    bool strip;
    string inFileName;
    string outFileName;

    CmdLineOpts(unsigned int argc, char* argv[]);

    void Usage();
};


CmdLineOpts::CmdLineOpts(unsigned int argc, char* argv[])
{
    progName = argv[0];

    if (argc < 1)
    {
        Usage();
        throw InvalidOptionsException();
    }

    strip = false;

    for (unsigned int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-s") == 0)
            {
                strip = true;
            }
            else if (strcmp(argv[i], "-i") == 0)
            {
                i++;
                inFileName = argv[i];
            }
            else if (strcmp(argv[i], "-o") == 0)
            {
                i++;
                outFileName = argv[i];
            }
            else if (strcmp(argv[i], "-h") == 0)
            {
                Usage();
                throw InvalidOptionsException();
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
}


void CmdLineOpts::Usage()
{
    cerr << endl << "Usage:" << endl << endl;
    cerr << progName << endl;
    cerr << "  [-s] [-i <input file>] [-o <output file>]" << endl;
}


void non_printable(std::istream& inStream, std::ostream& outStream, bool strip)
{
    string str;

    unsigned int lineNo = 1;

    while (!inStream.eof())
    {
        if (strip && (lineNo != 1))
        {
            outStream << endl;
        }
 
        std::getline(inStream, str);

        for (unsigned int charI = 0; charI < str.size(); ++charI)
        {
            if (Char::IsCarriageReturn(str[charI]))
            {
                cerr << "ERROR: Windows carriage return in line# " << lineNo \
                  << ", position# " << charI + 1 << endl;

                if (strip)
                {
                    str.erase(charI, 1);
                }

                continue;
            }

            if (!Char::IsPrintable(str[charI]))
            {
                string asciiHexString;
                Char::AsciiCodeInHex(str[charI], asciiHexString);
                cerr << "ERROR: Non-printable character in line# " << lineNo \
                  << ", position# " << charI + 1 << ", hex code 0x" \
                  << asciiHexString << endl;

                if (strip)
                {
                    str.erase(charI, 1);
                }
            }
        }

        if (strip)
        {
            outStream << str;
        }

        ++lineNo;
    }
}


int main(int argc, char *argv[])
{
    try
    {
        CmdLineOpts opts(argc, argv);

        istream* inStreamP = &cin;
        ostream* outStreamP = &cout;

        ifstream inFile;
        if (!opts.inFileName.empty())
        {
            inFile.open(opts.inFileName.c_str());
            inStreamP = &inFile;
        }

        ofstream outFile;
        if (opts.strip && !opts.outFileName.empty())
        {
            outFile.open(opts.outFileName.c_str());
            outStreamP = &outFile;
        }
 
        non_printable(*inStreamP, *outStreamP, opts.strip);

        if (!opts.inFileName.empty())
        {
            inFile.close();
        }

        if (opts.strip && !opts.outFileName.empty())
        {
            outFile.close();
            outStreamP = &outFile;
        }

        return (0);
    }
    catch (const exception& exc)
    {
        cerr << exc.what();

        return (1);
    }
}

