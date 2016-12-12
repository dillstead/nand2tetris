#include <iostream>
#include <fstream>
using namespace std;

#include "analyzer.h"
#include "compilation_engine.h"

bool Analyzer::Analyze(const string &in_filename)
{
    ifstream infile;

    cout << "Analyzing " << in_filename << endl;

    infile.open(in_filename.c_str(), ios::in);
    if (infile.fail())
    {
        cerr << "Error reading " << in_filename << endl;
        return false;
    }

    string out_filename = in_filename.substr(0, in_filename.find_last_of('.'));
    out_filename.append(".xml");

    ofstream outfile;

    outfile.open(out_filename.c_str(), ios::out);
    if (outfile.fail())
    {
        cerr << "Error writing " << out_filename << endl;
        return false;
    }

    CompilationEngine compilation_engine(infile, outfile);

    return compilation_engine.Compile();
}
