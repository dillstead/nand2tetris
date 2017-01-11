#include <iostream>

#include "compilation_engine.h"

// category
// var, argument, static, field, class, subroutine
// action
// define, use for variables
void CompilationEngine::XmlWriter::OpenElement(const char *element_name)
{
    Indent();
    out_ <<  "<" << element_name << ">" << endl;
    //cout <<  "<" << element_name << ">" << endl;
    indentation_++;
}

void CompilationEngine::XmlWriter::CloseElement(const char *element_name)
{
    indentation_--;
    Indent();
    out_ <<  "</" << element_name << ">" << endl;
    //cout <<  "</" << element_name << ">" << endl;
}

void CompilationEngine::XmlWriter::WriteIdentifierElement(const string &name, const string &kind)
{
    OpenElement("identifier");
    Indent();
    out_ << "<name> " << name << " </name>" << endl;
    Indent();
    out_ << "<kind> " << kind << " </kind>" << endl;
    CloseElement("identifier");
}

void CompilationEngine::XmlWriter::WriteIdentifierElement(const string &name, const string &kind, const string &action, int index)
{
    OpenElement("identifier");
    Indent();
    out_ << "<name> " << name << " </name>" << endl;
    Indent();
    out_ << "<kind> " << kind << " </kind>" << endl;
    Indent();
    out_ << "<action> " << action << " </action>" << endl;
    Indent();
    out_ << "<index> " << index << " </index>" << endl;
    CloseElement("identifier");
}

void CompilationEngine::XmlWriter::WriteTokenElement(const Tokenizer &tokenizer)
{
    Indent();
    switch (tokenizer.tokentype())
    {
    case TokenType::IDENTIFIER:
    {
        out_ << "<identifier> " << tokenizer.identifier() << " </identifier>" << endl;
        //cout << "<identifier> " << tokenizer.identifier() << " </identifier>" << endl;
        break;
    }
    case TokenType::INT_CONST:
    {
        out_ << "<integerConstant> " << tokenizer.intval() << " </integerConstant>" << endl;
        //cout << "<integerConstant> " << tokenizer.intval() << " </integerConstant>" << endl;
        break;
    }
    case TokenType::KEYWORD:
    {
        out_ << "<keyword> " << KeywordName(tokenizer.keyword()) << " </keyword>" << endl;
        //cout << "<keyword> " << KeywordName(tokenizer.keyword()) << " </keyword>" << endl;
        break;
    }
    case TokenType::STRING_CONST:
    {
        out_ << "<stringConstant> " << tokenizer.stringval() << " </stringConstant>" << endl;
        //cout << "<stringConstant> " << tokenizer.stringval() << " </stringConstant>" << endl;
        break;
    }
    case TokenType::SYMBOL:
    {
        switch (tokenizer.symbol())
        {
        case '>':
        {
            out_ << "<symbol> &gt; </symbol>" << endl;
            //cout << "<symbol> &gt; </symbol>" << endl;
            break;
        }
        case '<':
        {
            out_ << "<symbol> &lt; </symbol>" << endl;
            //cout << "<symbol> &lt; </symbol>" << endl;
            break;
        }
        case '&':
        {
            out_ << "<symbol> &amp; </symbol>" << endl;
            //cout << "<symbol> &amp; </symbol>" << endl;
            break;
        }
        default:
        {
            out_ << "<symbol> " << tokenizer.symbol() << " </symbol>" << endl;
            //cout << "<symbol> " << tokenizer.symbol() << " </symbol>" << endl;
            break;
        }
        }

        break;
    }
    default:
    {
        cerr << "Unknown token type" << endl;
        break;
    }
    }
}

void CompilationEngine::XmlWriter::Indent()
{
    for (int i = 0; i < indentation_; i++)
    {
        out_ <<  " ";
        //cout <<  " ";
    }
}

CompilationEngine::CompilationEngine(ifstream &in, ofstream &out) : tokenizer_(in), xmlwriter_(out),
        opset_({ '+',
        '-',
        '*',
        '/',
        '&',
        '|',
        '<',
        '>',
        '=' })
{

}

bool CompilationEngine::Compile()
{
    ChompToken();  // Arrive at first token
    if (!CompileClass())
    {
        goto error;
    }

    return true;

error:
    return false;
}

// Grammar rule
//   class:
//     'class' className '{' classVarDec* subroutineDec* '}'
//
//   className
//     identifier
bool CompilationEngine::CompileClass()
{
    if (!tokenizer_.HasMoreTokens())
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'class'" << endl;
        goto error;
    }

    xmlwriter_.OpenElement("class");

    if (tokenizer_.tokentype() == TokenType::KEYWORD && tokenizer_.keyword() == Keyword::CLASS)
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'class'" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected class name" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '{'"<< endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::STATIC || tokenizer_.keyword() == Keyword::FIELD))
    {
        if (!CompileClassVarDec())
        {
            goto error;
        }
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::CONSTRUCTOR || tokenizer_.keyword() == Keyword::FUNCTION
                || tokenizer_.keyword() == Keyword::METHOD))
    {
        if (!CompileSubroutine())
        {
            goto error;
        }
    }

    if (tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '}'"<< endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens())
    {
        cerr << "Expected one class per file" << endl;
        goto error;
    }

    xmlwriter_.CloseElement("class");

    return true;

 error:
    return false;
}

// Grammar rule
//   classVarDec
//     ('static' | 'field') type varName (',' varName)* ';'
//
//   type
//     'int' | 'char' | 'boolean' | className
//
//   className
//     identifier
//
//   varName
//     identifier
bool CompilationEngine::CompileClassVarDec()
{
    string type;

    xmlwriter_.OpenElement("classVarDec");

    // Token has already been checked and will be static or field.
    bool isStatic = (tokenizer_.keyword() == Keyword::STATIC);
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    bool isClassName = false;
    if (tokenizer_.HasMoreTokens())
    {
        if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
            isClassName = true;
        }
        else if  (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean' or class name" << endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean' or class name" << endl;
        goto error;
    }

    type = isClassName ? tokenizer_.identifier() : KeywordName(tokenizer_.keyword());
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        symbol_table_.Define(tokenizer_.identifier(), type, isStatic ? SymbolKind::STATIC : SymbolKind::FIELD);
        xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                symbol_table_.IndexOf(tokenizer_.identifier()));
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            symbol_table_.Define(tokenizer_.identifier(), type, isStatic ? SymbolKind::STATIC : SymbolKind::FIELD);
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                    symbol_table_.IndexOf(tokenizer_.identifier()));
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
            goto error;
        }
    }


    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": ';'" << endl;
        goto error;
    }

    xmlwriter_.CloseElement("classVarDec");

    return true;

error:
    return false;
}

// Grammar rule
//   subroutineDec
//     ('constructor' | 'function' | 'method') ('void | type) subroutineName
//     '(' parameterList ')' subroutineBody
//
//   type
//     'int' | 'char' | 'boolean' | className
//
//   className
//     identifier
//
//   subroutineName
//     identifier
bool CompilationEngine::CompileSubroutine()
{
    xmlwriter_.OpenElement("subroutineDec");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens())
    {
        if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
        }
        else if  (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN
            || tokenizer_.keyword() == Keyword::VOID))
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean', 'void' or class name" << endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean', 'void' or class name" << endl;
        goto error;
    }
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "subroutine");
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected subroutine name" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileParameterList())
        {
            goto error;
        }

    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected identifier '('" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected identifier ')'" << endl;
        goto error;
    }

    if (!CompileSubroutineBody())
    {
        goto error;
    }

    xmlwriter_.CloseElement("subroutineDec");

    return true;

error:
    return false;
}

// Grammar rule
//    subroutineBody
//     '{' varDec* statements '}'
bool CompilationEngine::CompileSubroutineBody()
{
    xmlwriter_.OpenElement("subroutineBody");

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '{'" << endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::KEYWORD && tokenizer_.keyword() == Keyword::VAR)
    {
        if (!CompileVarDec())
        {
            goto error;
        }
    }

    if (!CompileStatements())
    {
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected identifier '}'" << endl;
        goto error;
    }

    xmlwriter_.CloseElement("subroutineBody");

    return true;

error:
    return false;
}

// Grammar rule
//   parameterList
//     (type varName (',' type varName)*)?
//
//   type
//     'int' | 'char' | 'boolean' | className
//
//   className
//     identifier
//
//   varName
//     identifier
bool CompilationEngine::CompileParameterList()
{
    string type;

    xmlwriter_.OpenElement("parameterList");

    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ')'))
    {
        // Non-empty parameterList
        if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
            type = tokenizer_.identifier();
        }
        else if (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            type = KeywordName(tokenizer_.keyword());
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean' or class name" << endl;
            goto error;
        }
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::ARG);
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                symbol_table_.IndexOf(tokenizer_.identifier()));
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
            goto error;
        }

        while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();

            if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
            {
                xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
                type = tokenizer_.identifier();
            }
            else if (tokenizer_.tokentype() == TokenType::KEYWORD
                && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
            {
                xmlwriter_.WriteTokenElement(tokenizer_);
                type = KeywordName(tokenizer_.keyword());
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean' or class name" << endl;
                goto error;
            }
            ChompToken();

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
            {
                symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::ARG);
                xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                        symbol_table_.IndexOf(tokenizer_.identifier()));
                ChompToken();
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
                goto error;
            }
        }
    }

    xmlwriter_.CloseElement("parameterList");

    return true;

error:
    return false;
}

// Grammar rule
//   varDec
//     'var' type varName (',' varName)* ';'
//
//   type
//     'int' | 'char' | 'boolean' | className
//
//   className
//     identifier
//
//   varName
//     identifier
bool CompilationEngine::CompileVarDec()
{
    string type;

    xmlwriter_.OpenElement("varDec");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
        type = tokenizer_.identifier();
    }
    else if (tokenizer_.tokentype() == TokenType::KEYWORD
        && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        type = KeywordName(tokenizer_.keyword());
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'int', 'char', 'boolean' or class name" << endl;
        goto error;
    }
    ChompToken();


    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::VAR);
        xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                symbol_table_.IndexOf(tokenizer_.identifier()));
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::VAR);
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "define",
                symbol_table_.IndexOf(tokenizer_.identifier()));
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
            goto error;
        }
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": ';'" << endl;
        goto error;
    }

    xmlwriter_.CloseElement("varDec");

    return true;

error:
    return false;
}

// Grammar rule
//   statements:
//     statement*
//
//   statement:
//     letStatement | ifStatement | whileStatement
//     | doStatement | returnStatement
bool CompilationEngine::CompileStatements()
{
    xmlwriter_.OpenElement("statements");

    while (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != '}'))
    {
        if (tokenizer_.tokentype() == TokenType::KEYWORD)
        {
            if (tokenizer_.keyword() == Keyword::LET)
            {
                if (!CompileLet())
                {
                    goto error;
                }
            }
            else if (tokenizer_.keyword() == Keyword::IF)
            {
                if (!CompileIf())
                {
                    goto error;
                }
            }
            else if (tokenizer_.keyword() == Keyword::WHILE)
            {
                if (!CompileWhile())
                {
                    goto error;
                }
            }
            else if (tokenizer_.keyword() == Keyword::DO)
            {
                if (!CompileDo())
                {
                    goto error;
                }
            }
            else if (tokenizer_.keyword() == Keyword::RETURN)
            {
                if (!CompileReturn())
                {
                    goto error;
                }
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected 'let', 'if', 'while', 'do' or 'return'" << endl;
                goto error;
            }
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected 'let', 'if', 'while', 'do' or 'return'" << endl;
            goto error;
        }
    }

    xmlwriter_.CloseElement("statements");

    return true;

error:
    return false;
}

// Grammar rule
//   doStatement:
//     'do' subroutineCall ';'
bool CompilationEngine::CompileDo()
{
    xmlwriter_.OpenElement("doStatement");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        string identifier(tokenizer_.identifier());

        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && (tokenizer_.symbol() == '('
            || tokenizer_.symbol() == '.'))
        {
            if (!CompileSubroutineCall(identifier))
            {
                goto error;
            }
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '(' or '.'" << endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected subroutine name" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected ';'"<< endl;
        goto error;
    }

    xmlwriter_.CloseElement("doStatement");

    return true;

error:
    return false;
}

// Grammar rule
//   letStatement:
//     'let' varName ('[' expression ']')? '=' expression ';'
bool CompilationEngine::CompileLet()
{
    xmlwriter_.OpenElement("letStatement");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        string identifier(tokenizer_.identifier());

        ChompToken();

        if (tokenizer_.HasMoreTokens())
        {
            if (tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '[')
            {
                if (!CompileArrayIndex(identifier))
                {
                    goto error;
                }
            }
            else
            {
                xmlwriter_.WriteIdentifierElement(identifier, SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "use",
                        symbol_table_.IndexOf(identifier));
            }
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '=')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);

            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
            {
                xmlwriter_.WriteTokenElement(tokenizer_);
                ChompToken();
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected ';'"<< endl;
                goto error;
            }
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '[', or '='"<< endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected variable name"<< endl;
        goto error;
    }

    xmlwriter_.CloseElement("letStatement");

    return true;

error:
    return false;
}

// Grammar rule
//   whileStatement:
//     'while' '(' expression ')' '{' statements '}'
bool CompilationEngine::CompileWhile()
{
    xmlwriter_.OpenElement("whileStatement");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileExpression())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '('"<< endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileStatements())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '}'"<< endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '{'"<< endl;
        goto error;
    }

    xmlwriter_.CloseElement("whileStatement");

    return true;

error:
    return false;
}

// Grammar rule
//   returnStatement:
//     'return' expression? ';'
bool CompilationEngine::CompileReturn()
{
    xmlwriter_.OpenElement("returnStatement");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ';'))
    {
        if (!CompileExpression())
        {
            goto error;
        }
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected ';'" << endl;
        goto error;
    }

    xmlwriter_.CloseElement("returnStatement");

    return true;

error:
    return false;
}

// Grammar rule
//   ifStatement:
//     'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?
bool CompilationEngine::CompileIf()
{
    xmlwriter_.OpenElement("ifStatement");

    // Token has already been checked.
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileExpression())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '('"<< endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileStatements())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '}'"<< endl;
            goto error;
        }
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected '{'"<< endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::KEYWORD && tokenizer_.keyword() == Keyword::ELSE)
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();

            if (!CompileStatements())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
            {
                xmlwriter_.WriteTokenElement(tokenizer_);
                ChompToken();
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected '}'"<< endl;
                goto error;
            }
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '{'"<< endl;
            goto error;
        }
    }

    xmlwriter_.CloseElement("ifStatement");

    return true;

error:
    return false;
}

// Grammar rule
//   expression:
//     term (op term)*
//
//   op
//     '+' | '-' | '*'| '/' | '&' | '|' | '<' | '>' | '='
bool CompilationEngine::CompileExpression()
{
    xmlwriter_.OpenElement("expression");

    if (!CompileTerm())
    {
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && opset_.count(tokenizer_.symbol()) > 0)
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileTerm())
        {
            goto error;
        }
    }

    xmlwriter_.CloseElement("expression");

    return true;

error:
    return false;
}

// Grammar rule
//   term:
//     integerConstant | stringConstant | keywordConstant
//     | varName | varName '[' expression ']' | subroutineCall
//     | '(' expression ')' | unaryOp term
//
//   keywordConstant
//     'true' | 'false' | 'null' | 'this'
//
//   varName
//     identifier
//
//   unaryOp
//     '-' | '~'
bool CompilationEngine::CompileTerm()
{
    if (!tokenizer_.HasMoreTokens())
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected integer, string, 'true', 'false', 'null', "
                "this, '(', '-', or '~'" << endl;
        goto error;
    }

    xmlwriter_.OpenElement("term");

    if (tokenizer_.tokentype() == TokenType::INT_CONST
            || tokenizer_.tokentype() == TokenType::STRING_CONST)
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else if (tokenizer_.tokentype() == TokenType::KEYWORD)
    {
        if (tokenizer_.keyword() == Keyword::TRUE || tokenizer_.keyword() == Keyword::FALSE
                || tokenizer_.keyword() == Keyword::NULLZ || tokenizer_.keyword() == Keyword::THIS)
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected 'true', 'false', 'null' or 'this'"<< endl;
            goto error;
        }
    }
    else if (tokenizer_.tokentype() == TokenType::SYMBOL)
    {
        if (tokenizer_.symbol() == '(')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
            {
                xmlwriter_.WriteTokenElement(tokenizer_);
                ChompToken();
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
                goto error;
            }
        }
        else if (tokenizer_.symbol() == '-' || tokenizer_.symbol() == '~')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();

            if (!CompileTerm())
            {
                goto error;
            }
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '(', '-', or '~'" << endl;
            goto error;
        }
    }
    else
    {
        // Must be an identifier.
        string identifier(tokenizer_.identifier());
        ChompToken();

        if (tokenizer_.HasMoreTokens())
        {
            if (tokenizer_.tokentype() == TokenType::SYMBOL  && tokenizer_.symbol() == '[')
            {
                if (!CompileArrayIndex(identifier))
                {
                    goto error;
                }
            }
            else if (tokenizer_.tokentype() == TokenType::SYMBOL  && (tokenizer_.symbol() == '('
                    || tokenizer_.symbol() == '.'))
            {
                if (!CompileSubroutineCall(identifier))
                {
                    goto error;
                }
            }
            else
            {
                // variable
                xmlwriter_.WriteIdentifierElement(identifier, SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "use",
                        symbol_table_.IndexOf(identifier));
            }
        }
        else
        {
            // variable
            xmlwriter_.WriteIdentifierElement(identifier, SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "use",
                    symbol_table_.IndexOf(identifier));
        }
    }

    xmlwriter_.CloseElement("term");

    return true;

error:
    return false;
}

// Grammar rule
//   expressionList:
//     (expression (',', expression)*)?
bool CompilationEngine::CompileExpressionList()
{
    xmlwriter_.OpenElement("expressionList");

    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ')'))
    {
        // Non-empty expressionList
        if (!CompileExpression())
        {
            goto error;
        }

        while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }
        }
    }

    xmlwriter_.CloseElement("expressionList");

    return true;

error:
    return false;
}

bool CompilationEngine::CompileArrayIndex(const string &identifier)
{
    // Identifier is varName, current token is '['.
    xmlwriter_.WriteIdentifierElement(identifier, SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())), "use",
            symbol_table_.IndexOf(identifier));
    xmlwriter_.WriteTokenElement(tokenizer_);
    ChompToken();

    if (!CompileExpression())
    {
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ']')
    {
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected ']'"<< endl;
        goto error;
    }

    return true;

error:
    return false;
}

// Grammar rule
//   subroutineCall:
//     subroutineName '(' expressionList ')'
//     | (className | varName) '.' subroutineName '(' expressionList ')'
bool CompilationEngine::CompileSubroutineCall(const string &identifier)
{
    if (tokenizer_.symbol() == '(')
    {
        // Identifier is subroutineName.
        xmlwriter_.WriteIdentifierElement(identifier, "subroutine");
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (!CompileExpressionList())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
            goto error;
        }
    }
    else if (tokenizer_.symbol() == '.')
    {
        // Identifier is className or varName.
        if (symbol_table_.KindOf(tokenizer_.identifier()) == SymbolKind::NONE)
        {
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "class");
        }
        else
        {
            xmlwriter_.WriteIdentifierElement(identifier, "var", SymbolKindName(symbol_table_.KindOf(tokenizer_.identifier())),
                    symbol_table_.IndexOf(identifier));
        }
        xmlwriter_.WriteTokenElement(tokenizer_);
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            xmlwriter_.WriteIdentifierElement(tokenizer_.identifier(), "subroutine");
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected subroutine name"<< endl;
        }
        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '('"<< endl;
            goto error;
        }

        if (!CompileExpressionList())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
            xmlwriter_.WriteTokenElement(tokenizer_);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
            goto error;
        }
    }

    return true;

error:
    return false;
}
