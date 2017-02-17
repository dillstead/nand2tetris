#ifndef COMPILATION_ENGINE_H_
#define COMPILATION_ENGINE_H_

#include <fstream>
#include <unordered_map>
#include <set>

#include "utils.h"
#include "tokenizer.h"
#include "symbol_table.h"
#include "vm_writer.h"

typedef set<char> OperatorSet;
typedef unordered_map<SymbolKind, Segment, EnumClassHash> SymbolKindMap;

typedef struct ClassInfo
{
    string name;
} ClassInfo;

typedef struct SubroutineInfo
{
    // Constructor, method, or function.
    Keyword type;
    string name;
    int num_returns;
    bool void_return;
} SubroutineInfo;

// Effects actual Jack compilation.  Gets input from the Tokenizer and emits its parsed structure
// into an output file.  The output is generated by a series of CompileXxx() routines, one for every
// syntatic element, Xxx, of the Jack grammar.  The contract between these routines is that each
// CompileXxx() rountine should read the syntactic construct Xxx from the input, advance the
// Tokenizer exactly beyond Xxx, and output the parsing of Xxx.

class CompilationEngine
{
    bool ChompToken()
    {
        return (tokenizer_.HasMoreTokens() && tokenizer_.Advance());
    }

    // All CompileXxx routines return false if the grammar is not properly matched.
    bool CompileClass();
    bool CompileClassVarDec();
    bool CompileSubroutine();
    bool CompileSubroutineBody();
    bool CompileParameterList();
    bool CompileVarDec();
    bool CompileStatements();
    bool CompileDo();
    bool CompileLet();
    bool CompileWhile();
    bool CompileReturn();
    bool CompileIf();
    bool CompileExpression();
    bool CompileTerm();
    bool CompileExpressionList(int &num_args);
    bool CompileArrayIndex(const string &identifier);
    bool CompileSubroutineCall(const string &identifier);

    const string MakeSubroutineName(const string &subroutine_name)
    {
        return class_info_.name + "." + subroutine_name;
    }

    const string MakeSubroutineName(const string &class_name, const string &subroutine_name)
    {
        return class_name + "." + subroutine_name;
    }

    const string MakeLabel(const string &subroutine_name)
    {
        return class_info_.name + "." + subroutine_name + "." + to_string((long long)num_labels++);
    }

    // The following methods provide a higher level abstraction to VMWriter and are used
    // to write the VM code.
    void WriteOperation(int op);
    void WriteSubroutineSetup();
    bool WriteStoreRValue(const string &lvalue, bool is_array);
    bool WriteValue(const string &value, bool is_array);
    void WriteIntegerConstant(int int_const);
    void WriteStringConstant(const string &string_const);
    bool WriteKeywordConstant(Keyword keyword_const);

    Tokenizer tokenizer_;
    SymbolTable symbol_table_;
    VmWriter vm_writer_;
    // Tracks the number of labels to ensure new labels are unique.
    int num_labels;
    OperatorSet op_set_;
    SymbolKindMap symbolkind_map_;
    // Information about the current class, subroutine, and var being compiled.
    ClassInfo class_info_;
    SubroutineInfo subroutine_info_;

public:
    // Creates a tokenizer with a input stream ready for reading and an output stream for
    // writing.
    CompilationEngine(ifstream &in, ofstream &out);
    // Performs the compilation.
    bool Compile();
};

#endif
