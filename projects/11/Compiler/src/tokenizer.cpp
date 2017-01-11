#include <ctype.h>
#include <iostream>
#include <vector>

#include "tokenizer.h"

static vector<string> keyword_names = {
    "class",
    "method",
    "function",
    "constructor",
    "int",
    "boolean",
    "char",
    "void",
    "var",
    "static",
    "field",
    "let",
    "do",
    "if",
    "else",
    "while",
    "return",
    "true",
    "false",
    "null",
    "this"
};

const string &KeywordName(Keyword keyword)
{
    return keyword_names[int(keyword)];
};

Tokenizer::Tokenizer(ifstream &in) : in_(in), state_(READING_WHITESPACE),
    keywordmap_({ { "class", Keyword::CLASS },
    { "method", Keyword::METHOD },
    { "function", Keyword::FUNCTION },
    { "constructor", Keyword::CONSTRUCTOR },
    { "int", Keyword::INT },
    { "boolean", Keyword::BOOLEAN },
    { "char", Keyword::CHAR },
    { "void", Keyword::VOID },
    { "var", Keyword::VAR },
    { "static", Keyword::STATIC },
    { "field", Keyword::FIELD },
    { "let", Keyword::LET },
    { "do", Keyword::DO },
    { "if", Keyword::IF },
    { "else", Keyword::ELSE },
    { "while", Keyword::WHILE },
    { "return", Keyword::RETURN },
    { "true", Keyword::TRUE },
    { "false", Keyword::FALSE },
    { "null", Keyword::NULLZ },
    { "this", Keyword::THIS } }),
    symbolset_({ '{',
    '}',
    '(',
    ')',
    '[',
    ']',
    '.',
    ',',
    ';',
    '+',
    '-',
    '*',
    '/',
    '&',
    '|',
    '<',
    '>',
    '=',
    '~' })
{

}

bool Tokenizer::HasMoreTokens()
{
    int c;
    int prevc = '\0';

    while (state_ != READING_TOKEN && (c = in_.get()) != EOF)
    {
        switch (state_)
        {
        case READING_WHITESPACE:
        {
            if (!isspace(c))
            {
                if (c == '/')
                {
                    state_ = MAYBE_READING_COMMENT;
                }
                else
                {
                    in_.putback(c);
                    state_ = READING_TOKEN;
                }
            }
            break;
        }
        case MAYBE_READING_COMMENT:
        {
            if (c == '/')
            {
                state_ = READING_SINGLE_LINE_COMMENT;
            }
            else if (c == '*')
            {
                state_ = READING_MULTI_LINE_COMMENT;
            }
            else
            {
                in_.putback(c).putback('/');
                state_ = READING_TOKEN;
            }
            break;
        }
        case READING_SINGLE_LINE_COMMENT:
        {
            if (c == '\n')
            {
                state_ = READING_WHITESPACE;
            }
            break;
        }
        case READING_MULTI_LINE_COMMENT:
        {
            if (prevc == '*' && c == '/')
            {
                state_ = READING_WHITESPACE;
            }
            break;
        }
        default:
        {
            cerr << "Invalid internal state" << endl;
            break;
        }
        }

        prevc = c;
    }

    return (state_ == READING_TOKEN);
}

bool Tokenizer::Advance()
{
    if (state_ != READING_TOKEN)
    {
        cerr << "Invalid state" << endl;
        return false;
    }

    // Guaranteed to not be EOF, pushed back in HasMoreTokens.
    int c = in_.get();

    if (symbolset_.count(c) > 0)
    {
        symbol_ = c;
        tokentype_ = TokenType::SYMBOL;
        state_ = READING_WHITESPACE;
    }
    else if (isdigit(c))
    {
        intval_ = c - '0';
        state_ = READING_INT_CONST;
    }
    else if (c == '"')
    {
        stringval_.clear();
        state_ = READING_STRING_CONST;
    }
    else if (isalpha(c) || c == '_')
    {
        // Reading keyword or identifier.
        // An identifier is a sequence of letters, digits, and underscores not
        // starting with a digit.
        token_.clear();
        token_.push_back(c);
    }
    else
    {
        cerr << "Line " << in_.lineno() << ": Invalid character " << c << endl;
        return false;
    }

    bool valid = true;

    while (state_ != READING_WHITESPACE)
    {
        c = in_.get();

        switch (state_)
        {
        case READING_INT_CONST:
        {
            if (isdigit(c))
            {
                intval_ *= 10;
                intval_ += c - '0';
            }
            else
            {
                // Integer constants are in the range 0..32767.
                if (intval_ > 32767)
                {
                    cerr << "Line " << in_.lineno() << ": Integer constant too large" << endl;
                    valid = false;
                }
                in_.putback(c);
                tokentype_ = TokenType::INT_CONST;
                state_ = READING_WHITESPACE;
            }
            break;
        }
        case READING_STRING_CONST:
        {
            if (c == '"' || c == EOF)
            {
                if (c == EOF)
                {
                    // String constant must end with '"'.
                    cerr << "Line " << in_.lineno() << ": Unterminated string constant" << endl;
                    valid = false;
                }
                tokentype_ = TokenType::STRING_CONST;
                state_ = READING_WHITESPACE;
            }
            else
            {
                stringval_.push_back(c);
            }
            break;
        }
        case READING_TOKEN:
        {
            if (isalpha(c) || isdigit(c) || c == '_')
            {
                token_.push_back(c);
            }
            else
            {
                in_.putback(c);
                if (keywordmap_.count(token_) > 0)
                {
                    keyword_ = keywordmap_[token_];
                    tokentype_ = TokenType::KEYWORD;
                }
                else
                {
                    tokentype_ = TokenType::IDENTIFIER;
                }
                state_ = READING_WHITESPACE;
            }
            break;
        }
        default:
        {
            cerr << "Invalid internal state" << endl;
            valid = false;
            break;
        }
        }
    }

    //if (valid)
    //{
    //    DumpTokenInfo();
    //}

    return valid;
}

void Tokenizer::DumpTokenInfo()
{
    switch (tokentype_)
    {
        case TokenType::IDENTIFIER:
        {
            cout << "identifier: " << token_ << endl;
            break;
        }
        case TokenType::INT_CONST:
        {
            cout << "integerConstant: " << intval_ << endl;
            break;
        }
        case TokenType::KEYWORD:
        {
            cout << "keyword: " << token_ << endl;
            break;
        }
        case TokenType::STRING_CONST:
        {
            cout << "stringConstant: " << stringval_ << endl;
            break;
        }
        case TokenType::SYMBOL:
        {
            cout << "symbol: " << symbol_ << endl;
            break;
        }
        default:
        {
            cerr << "Unknown token type" << endl;
            break;
        }
        }
}
