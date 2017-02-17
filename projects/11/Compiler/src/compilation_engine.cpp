#include <iostream>

#include "compilation_engine.h"

static const string kMemoryAlloc = "Memory.alloc";
static const string kStringNew = "String.new";
static const string kStringAppendChar = "String.appendChar";
static const string kMultiply = "Math.multiply";
static const string kDivide = "Math.divide";

CompilationEngine::CompilationEngine(ifstream &in, ofstream &out) : tokenizer_(in), vm_writer_(out), num_labels(0),
        op_set_({ '+',
        '-',
        '*',
        '/',
        '&',
        '|',
        '<',
        '>',
        '=' }),
        symbolkind_map_({ { SymbolKind::ARGUMENT, Segment::ARGUMENT },
        { SymbolKind::FIELD, Segment::THIS },
        { SymbolKind::STATIC, Segment::STATIC },
        { SymbolKind::VARIABLE, Segment::LOCAL } })
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


    if (tokenizer_.tokentype() == TokenType::KEYWORD && tokenizer_.keyword() == Keyword::CLASS)
    {
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected 'class'" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        class_info_.name= tokenizer_.identifier();
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected class name" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
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

    // Token has already been checked and will be static or field.
    bool isStatic = (tokenizer_.keyword() == Keyword::STATIC);
    ChompToken();

    bool isClassName = true;
    if (tokenizer_.HasMoreTokens())
    {
        if  (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
        {
            isClassName = false;
        }
        else if (tokenizer_.tokentype() != TokenType::IDENTIFIER)
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
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
    {
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            symbol_table_.Define(tokenizer_.identifier(), type, isStatic ? SymbolKind::STATIC : SymbolKind::FIELD);
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
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": ';'" << endl;
        goto error;
    }

    return true;

error:
    return false;
}

// Grammar rule
//   subroutineDec
//     ('constructor' | 'function' | 'method') ('void' | type) subroutineName
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
    subroutine_info_.type = tokenizer_.keyword();
    subroutine_info_.void_return = false;
    subroutine_info_.num_returns = 0;
    num_labels = 0;

    // Open a new scope for the subroutine.
    symbol_table_.StartSubroutine();

    // Token has already been checked.
    ChompToken();

    if (tokenizer_.HasMoreTokens())
    {
        if  (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN
            || tokenizer_.keyword() == Keyword::VOID))
        {
            if (tokenizer_.keyword() == Keyword::VOID)
            {
                subroutine_info_.void_return = true;
            }
        }
        else if (tokenizer_.tokentype() != TokenType::IDENTIFIER)
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
        subroutine_info_.name = tokenizer_.identifier();
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected subroutine name" << endl;
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
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

    if (subroutine_info_.num_returns == 0)
    {
        cerr << "'" << subroutine_info_.name << "': No return found" << endl;
        return false;
    }

    return true;

error:
    return false;
}

// Grammar rule
//    subroutineBody
//     '{' varDec* statements '}'
bool CompilationEngine::CompileSubroutineBody()
{
    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
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

    WriteSubroutineSetup();

    if (!CompileStatements())
    {
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
    {
        ChompToken();

    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected identifier '}'" << endl;
        goto error;
    }

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

    // A method must be passed a "this" pointer as argument 0.
    if (subroutine_info_.type == Keyword::METHOD)
    {
        symbol_table_.Define("this", class_info_.name, SymbolKind::ARGUMENT);
    }

    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ')'))
    {
        // Non-empty parameterList
        if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            type = tokenizer_.identifier();
        }
        else if (tokenizer_.tokentype() == TokenType::KEYWORD
            && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
        {
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
            symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::ARGUMENT);
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
            goto error;
        }

        while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
        {
            ChompToken();

            if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
            {
                type = tokenizer_.identifier();
            }
            else if (tokenizer_.tokentype() == TokenType::KEYWORD
                && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
            {
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
                symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::ARGUMENT);
                ChompToken();
            }
            else
            {
                cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
                goto error;
            }
        }
    }

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

    // Token has already been checked.
    ChompToken();

    if (tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        type = tokenizer_.identifier();
    }
    else if (tokenizer_.tokentype() == TokenType::KEYWORD
        && (tokenizer_.keyword() == Keyword::INT || tokenizer_.keyword() == Keyword::CHAR || tokenizer_.keyword() == Keyword::BOOLEAN))
    {
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
        symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::VARIABLE);
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected variable name" << endl;
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
    {
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            symbol_table_.Define(tokenizer_.identifier(), type, SymbolKind::VARIABLE);
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
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": ';'" << endl;
        goto error;
    }

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

    return true;

error:
    return false;
}

// Grammar rule
//   doStatement:
//     'do' subroutineCall ';'
bool CompilationEngine::CompileDo()
{
    // Token has already been checked.
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

            // Return value (if any) is not going to be stored so discard it.
            vm_writer_.WritePop(Segment::TEMP, 0);
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
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected ';'"<< endl;
        goto error;
    }

    return true;

error:
    return false;
}

// Grammar rule
//   letStatement:
//     'let' varName ('[' expression ']')? '=' expression ';'
bool CompilationEngine::CompileLet()
{
    string identifier;
    bool store_into_array = false;

    // Token has already been checked.
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
    {
        identifier = tokenizer_.identifier();
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '[')
        {
            if (!CompileArrayIndex(identifier))
            {
                goto error;
            }
            store_into_array = true;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '=')
        {
            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
            {
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

    if (!WriteStoreRValue(identifier, store_into_array))
    {
        goto error;
    }

    return true;

error:
    return false;
}

// Grammar rule
//   whileStatement:
//     'while' '(' expression ')' '{' statements '}'
bool CompilationEngine::CompileWhile()
{
    const string &start_label = MakeLabel(subroutine_info_.name);
    const string &end_label = MakeLabel(subroutine_info_.name);

    // Token has already been checked.
    ChompToken();

    vm_writer_.WriteLabel(start_label);

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
        ChompToken();

        if (!CompileExpression())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
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

    vm_writer_.WriteArithmetic(Command::NOT);
    vm_writer_.WriteIf(end_label);

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
    {
        ChompToken();

        if (!CompileStatements())
        {
            goto error;
        }

        vm_writer_.WriteGoto(start_label);

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
        {
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

    vm_writer_.WriteLabel(end_label);

    return true;

error:
    return false;
}

// Grammar rule
//   returnStatement:
//     'return' expression? ';'
bool CompilationEngine::CompileReturn()
{
    subroutine_info_.num_returns++;

    // Token has already been checked.
    ChompToken();

    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ';'))
    {
        if (subroutine_info_.void_return)
        {
            cerr << "'" << MakeSubroutineName(subroutine_info_.name) << "': Cannot return a value" << endl;
            goto error;
        }
        if (!CompileExpression())
        {
            goto error;
        }
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ';')
    {
        ChompToken();
    }
    else
    {
        cerr << "Line " << tokenizer_.lineno() << ": Expected ';'" << endl;
        goto error;
    }

    // Void subroutines must return 0.
    if (subroutine_info_.void_return)
    {
        vm_writer_.WritePush(Segment::CONSTANT, 0);
    }

    vm_writer_.WriteReturn();

    return true;

error:
    return false;
}

// Grammar rule
//   ifStatement:
//     'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?
bool CompilationEngine::CompileIf()
{
    const string &not_cond_label = MakeLabel(subroutine_info_.name);
    const string &end_label = MakeLabel(subroutine_info_.name);

    // Token has already been checked.
    ChompToken();

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
    {
        ChompToken();

        if (!CompileExpression())
        {
            goto error;
        }

        vm_writer_.WriteArithmetic(Command::NOT);
        vm_writer_.WriteIf(not_cond_label);

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
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
        ChompToken();

        if (!CompileStatements())
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
        {
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

    vm_writer_.WriteGoto(end_label);
    vm_writer_.WriteLabel(not_cond_label);

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::KEYWORD && tokenizer_.keyword() == Keyword::ELSE)
    {
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '{')
        {
            ChompToken();

            if (!CompileStatements())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '}')
            {
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

    vm_writer_.WriteLabel(end_label);

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
    if (!CompileTerm())
    {
        goto error;
    }

    while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && op_set_.count(tokenizer_.symbol()) > 0)
    {
        char op = tokenizer_.symbol();
        ChompToken();

        if (!CompileTerm())
        {
            goto error;
        }

        WriteOperation(op);
    }

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

    if (tokenizer_.tokentype() == TokenType::STRING_CONST)
    {
        WriteStringConstant(tokenizer_.stringval());
        ChompToken();
    }
    else if (tokenizer_.tokentype() == TokenType::INT_CONST)
    {
        WriteIntegerConstant(tokenizer_.intval());
        ChompToken();
    }
    else if (tokenizer_.tokentype() == TokenType::KEYWORD)
    {
        if (tokenizer_.keyword() == Keyword::TRUE || tokenizer_.keyword() == Keyword::FALSE
                || tokenizer_.keyword() == Keyword::NULLZ || tokenizer_.keyword() == Keyword::THIS)
        {
            if (!WriteKeywordConstant(tokenizer_.keyword()))
            {
                goto error;
            }
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
            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }

            if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
            {
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
            char unaryOp = tokenizer_.symbol();

            ChompToken();

            if (!CompileTerm())
            {
                goto error;
            }

            unaryOp == '-' ? vm_writer_.WriteArithmetic(Command::NEG) : vm_writer_.WriteArithmetic(Command::NOT);
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
                if (!WriteValue(identifier, true))
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
                if (!WriteValue(identifier, false))
                {
                    goto error;
                }
            }
        }
        else
        {
            // variable
            if (!WriteValue(identifier, false))
            {
                goto error;
            }
        }
    }

    return true;

error:
    return false;
}

// Grammar rule
//   expressionList:
//     (expression (',', expression)*)?
bool CompilationEngine::CompileExpressionList(int &num_args)
{
    if (tokenizer_.HasMoreTokens() && (tokenizer_.tokentype() != TokenType::SYMBOL || tokenizer_.symbol() != ')'))
    {
        // Non-empty expressionList
        if (!CompileExpression())
        {
            goto error;
        }
        num_args++;

        while (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ',')
        {
            ChompToken();

            if (!CompileExpression())
            {
                goto error;
            }
            num_args++;
        }
    }

    return true;

error:
    return false;
}

bool CompilationEngine::CompileArrayIndex(const string &identifier)
{
    // Identifier is varName, current token is '['.
    ChompToken();

    if (!CompileExpression())
    {
        goto error;
    }

    if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ']')
    {
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
    string class_name;
    string subroutine_name;
    int num_args = 0;

    if (tokenizer_.symbol() == '(')
    {
        class_name = class_info_.name;
        subroutine_name = identifier;

        // Identifier is subroutineName.
        ChompToken();

        if (subroutine_info_.type == Keyword::FUNCTION)
        {
            cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Cannot call '" << subroutine_name << "' from function" << endl;
            goto error;
        }

        // Push this before any arguments.
        vm_writer_.WritePush(Segment::POINTER, 0);
        num_args++;

        if (!CompileExpressionList(num_args))
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
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
        if (symbol_table_.KindOf(identifier) == SymbolKind::NONE)
        {
            class_name = identifier;
        }
        else
        {
            class_name = symbol_table_.TypeOf(identifier);

            // Push var this before any arguments.
            WriteValue(identifier, false);
            num_args++;
        }
        ChompToken();

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::IDENTIFIER)
        {
            subroutine_name = tokenizer_.identifier();
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected subroutine name"<< endl;
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == '(')
        {
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected '('"<< endl;
            goto error;
        }

        if (!CompileExpressionList(num_args))
        {
            goto error;
        }

        if (tokenizer_.HasMoreTokens() && tokenizer_.tokentype() == TokenType::SYMBOL && tokenizer_.symbol() == ')')
        {
            ChompToken();
        }
        else
        {
            cerr << "Line " << tokenizer_.lineno() << ": Expected ')'"<< endl;
            goto error;
        }
    }

    vm_writer_.WriteCall(MakeSubroutineName(class_name, subroutine_name), num_args);

    return true;

error:
    return false;
}

void CompilationEngine::WriteOperation(int op)
{
    // Operate on the top two values on the stack.
    switch(op)
    {
    case '+':
    {
        vm_writer_.WriteArithmetic(Command::ADD);
        break;
    }
    case '-':
    {
        vm_writer_.WriteArithmetic(Command::SUB);
        break;
    }
    // No opcode support for multiply and divide, use the Math library instead.
    case '*':
    {
        vm_writer_.WriteCall(kMultiply, 2);
        break;
    }
    case '/':
    {
        vm_writer_.WriteCall(kDivide, 2);
        break;
    }
    case '&':
    {
        vm_writer_.WriteArithmetic(Command::AND);
        break;
    }
    case '|':
    {
        vm_writer_.WriteArithmetic(Command::OR);
        break;
    }
    case '<':
    {
        vm_writer_.WriteArithmetic(Command::LT);
        break;
    }
    case '>':
    {
        vm_writer_.WriteArithmetic(Command::GT);
        break;
    }
    case '=':
    {
        vm_writer_.WriteArithmetic(Command::EQ);
        break;
    }
    default:
    {
        cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Unknown operator '" << op << "'" << endl;
        break;
    }
    }
}

void CompilationEngine::WriteSubroutineSetup()
{
    vm_writer_.WriteFunction(MakeSubroutineName(subroutine_info_.name), symbol_table_.KindCount(SymbolKind::VARIABLE));

    if (subroutine_info_.type == Keyword::CONSTRUCTOR)
    {
        // Constructors must allocate memory for the fields in an object instance and setup the "this" segment to point
        // to the starting address of the object.
        vm_writer_.WritePush(Segment::CONSTANT, symbol_table_.KindCount(SymbolKind::FIELD));
        vm_writer_.WriteCall(kMemoryAlloc, 1);
        vm_writer_.WritePop(Segment::POINTER, 0);
    }
    else if (subroutine_info_.type == Keyword::METHOD)
    {
        // Methods must setup the "this" segment to point to the starting address of the object.
        // This address is passed in as the first argument to the method.
        vm_writer_.WritePush(Segment::ARGUMENT, 0);
        vm_writer_.WritePop(Segment::POINTER, 0);
    }
}

bool CompilationEngine::WriteStoreRValue(const string &lvalue, bool is_array)
{
    // If the lvalue is an array, there will be two elements of interest on the stack.  The top
    // value will be the rvalue and the next value will be the offset.  In the case that the lvalue
    // is not an array access, the top value will only be of interest.

    if (symbol_table_.KindOf(lvalue) == SymbolKind::NONE)
    {
        cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Unknown identifier '" << lvalue << "'" << endl;
        goto error;
    }

    if (is_array)
    {
        if (symbol_table_.TypeOf(lvalue) == KeywordName(Keyword::INT)
                || symbol_table_.TypeOf(lvalue) == KeywordName(Keyword::CHAR)
                || symbol_table_.TypeOf(lvalue) == KeywordName(Keyword::BOOLEAN))
        {
            cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Identifier '" << lvalue << "' cannot be indexed" << endl;
            goto error;
        }
        vm_writer_.WritePop(Segment::TEMP, 0);
        vm_writer_.WritePush(symbolkind_map_[symbol_table_.KindOf(lvalue)], symbol_table_.IndexOf(lvalue));
        vm_writer_.WriteArithmetic(Command::ADD);
        vm_writer_.WritePop(Segment::POINTER, 1);
        vm_writer_.WritePush(Segment::TEMP, 0);
        vm_writer_.WritePop(Segment::THAT, 0);
    }
    else
    {
        vm_writer_.WritePop(symbolkind_map_[symbol_table_.KindOf(lvalue)], symbol_table_.IndexOf(lvalue));
    }

    return true;

error:
    return false;
}

bool CompilationEngine::WriteValue(const string &value, bool is_array)
{
    // If the value is an array, the top of the stack will contain the offset.

    if (symbol_table_.KindOf(value) == SymbolKind::NONE)
    {
        cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Unknown identifier '" << value << "'" << endl;
        goto error;
    }

    if (is_array)
    {
        if (symbol_table_.TypeOf(value) == KeywordName(Keyword::INT)
                || symbol_table_.TypeOf(value) == KeywordName(Keyword::CHAR)
                || symbol_table_.TypeOf(value) == KeywordName(Keyword::BOOLEAN))
        {
            cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Identifier '" << value << "' cannot be indexed" << endl;
            goto error;
        }
        vm_writer_.WritePush(symbolkind_map_[symbol_table_.KindOf(value)], symbol_table_.IndexOf(value));
        vm_writer_.WriteArithmetic(Command::ADD);
        vm_writer_.WritePop(Segment::POINTER, 1);
        vm_writer_.WritePush(Segment::THAT, 0);
    }
    else
    {
        vm_writer_.WritePush(symbolkind_map_[symbol_table_.KindOf(value)], symbol_table_.IndexOf(value));
    }

    return true;

error:
    return false;

}

void CompilationEngine::WriteIntegerConstant(int int_const)
{
    vm_writer_.WritePush(Segment::CONSTANT, int_const);
}

void CompilationEngine::WriteStringConstant(const string &string_const)
{
    vm_writer_.WritePush(Segment::CONSTANT, string_const.length());
    vm_writer_.WriteCall(kStringNew, 1);
    for (unsigned int c = 0; c < string_const.length(); c++)
    {
        vm_writer_.WritePush(Segment::CONSTANT, string_const[c]);
        vm_writer_.WriteCall(kStringAppendChar, 2);
    }
}

bool CompilationEngine::WriteKeywordConstant(Keyword keyword_const)
{
    if (keyword_const == Keyword::NULLZ || keyword_const == Keyword::FALSE)
    {
        vm_writer_.WritePush(Segment::CONSTANT, 0);
    }
    else if (keyword_const == Keyword::THIS)
    {
        if (subroutine_info_.type == Keyword::FUNCTION)
        {
            cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Cannot refer to 'this'" << endl;
            goto error;
        }
        vm_writer_.WritePush(Segment::POINTER, 0);
    }
    else if (keyword_const == Keyword::TRUE)
    {
        // TRUE == -1 (1111 1111 1111 1111)
        vm_writer_.WritePush(Segment::CONSTANT, 0);
        vm_writer_.WriteArithmetic(Command::NOT);
    }
    else
    {
        cerr << "In '" << MakeSubroutineName(subroutine_info_.name) << "': Unknown keyword '" << KeywordName(keyword_const) << "'" << endl;
        goto error;
    }

    return true;

 error:
     return false;
}
