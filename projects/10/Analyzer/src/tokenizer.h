#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <fstream>
#include <unordered_map>
#include <set>
using namespace std;

enum class TokenType
{
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STRING_CONST
};

enum class Keyword
{
    CLASS,
    METHOD,
    FUNCTION,
    CONSTRUCTOR,
    INT,
    BOOLEAN,
    CHAR,
    VOID,
    VAR,
    STATIC,
    FIELD,
    LET,
    DO,
    IF,
    ELSE,
    WHILE,
    RETURN,
    TRUE,
    FALSE,
    NULLZ,
    THIS
};

typedef set<int> SymbolSet;
typedef unordered_map<string, Keyword> KeywordMap;

// Removes all comments and white space from a Jack file and breaks it up into tokens as
// specified by the Jack grammar.

class Tokenizer
{
    enum State
    {
        READING_WHITESPACE,
        MAYBE_READING_COMMENT,
        READING_SINGLE_LINE_COMMENT,
        READING_MULTI_LINE_COMMENT,
        READING_TOKEN,
        READING_INT_CONST,
        READING_STRING_CONST
    };

    class token_ifstream
    {
        ifstream &in_;
        int lineno_;

        public:
            token_ifstream(ifstream &in) : in_(in), lineno_(1)
            {

            }

            int get()
            {
                int c = in_.get();

                if (c == '\n')
                {
                    lineno_++;
                }

                return c;
            }

            token_ifstream &putback(char c)
            {
                in_.putback(c);

                return *this;
            }

            int lineno() const
            {
                return lineno_;
            }
    };

    token_ifstream in_;
    State state_;
    KeywordMap keywordmap_;
    SymbolSet symbolset_;

    string token_;
    TokenType tokentype_;
    Keyword keyword_;
    int intval_;
    char symbol_;
    string stringval_;

public:
    // Creates a tokenizer with a input stream ready for reading.
    Tokenizer(ifstream &in);

    // Returns true if there are more tokens in the input.
    bool HasMoreTokens();

    // Get the next token from the input and makes it the current token.
    // This should only be called if HasMoreTokens() returns true.
    // Initially there is no current token.  Returns true if the token
    // is valid.
    bool Advance();

    TokenType tokentype() const
    {
        return tokentype_;
    }

    // Returns the keyword which is the current token.  Should only be called
    // when tokenType() is KEYWORD.
    const string &keyword() const
    {
        return token_;
    }

    // Returns the character which is the current token.  Should only be called
    // when token type is SYMBOL.
    char symbol() const
    {
        return symbol_;
    }

    // Returns the identifier which is the current token.  Should only be called
    // when token type is IDENTIFIER.
    const string &identifier() const
    {
        return token_;
    }

    // Returns the integer value of current token.  Should only be called
    // when token type is INT_CONST.
    int intval() const
    {
        return intval_;
    }

    // Returns the string value of the current token.  Should only be called
    // when the token type is STRING_CONST.
    const string &stringval() const
    {
        return stringval_;
    }
};

#endif
