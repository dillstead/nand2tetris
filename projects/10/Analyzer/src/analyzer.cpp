#include <iostream>
#include <fstream>
using namespace std;

#include "analyzer.h"
#include "tokenizer.h"

bool Analyzer::Analyze(string &filename)
{
    ifstream infile;

    cout << "Analyzing " << filename << endl;

    infile.open(filename.c_str(), ios::in);
    if (infile.fail())
    {
        cerr << "Error reading " << filename << endl;
        return false;
    }

    Tokenizer tokenizer(infile);
    bool valid = true;

    while (tokenizer.HasMoreTokens() && (valid = tokenizer.Advance()))
    {
        switch (tokenizer.tokentype())
        {
        case TokenType::IDENTIFIER:
        {
            cout << "IDENTIFIER " << tokenizer.identifier() << endl;
            break;
        }
        case TokenType::INT_CONST:
        {
            cout << "INT_CONST " << tokenizer.intval() << endl;
            break;
        }
        case TokenType::KEYWORD:
        {
            cout << "KEYWORD " << tokenizer.keyword() << endl;
            break;
        }
        case TokenType::STRING_CONST:
        {
            cout << "STRING_CONST " << tokenizer.stringval() << endl;
            break;
        }
        case TokenType::SYMBOL:
        {
            cout << "SYMBOL " << tokenizer.symbol() << endl;
            break;
        }
        default:
        {
            cerr << "Unknown token type" << endl;
            valid = false;
            break;
        }
        }
    }
    return valid;
}
