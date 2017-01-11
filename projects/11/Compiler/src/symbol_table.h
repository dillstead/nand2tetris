#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <unordered_map>
using namespace std;

enum class SymbolKind
{
    NONE,
    STATIC,
    FIELD,
    ARG,
    VAR,
};

// Returns convenient name of SymbolKind as a string.
const string &SymbolKindName(SymbolKind kind);

// Symbol table associates identifiers with properties needed for compilation.  The type (int, boolean, class etc.),
// the kind (argument, local variable, static, field), and an index for each kind starting with 0.  Two nested scopes
// are supported - class and subroutine.

class SymbolTable
{
    class Symbol
    {
public:
        string name_;
        string type_;
        SymbolKind kind_;
        int index_;

        Symbol()
        {

        }

        Symbol(const string &name, const string &type, SymbolKind kind, int index) : name_(name), type_(type), kind_(kind),
                index_(index)
        {

        }
    };

    typedef unordered_map<string, SymbolTable::Symbol> SymbolMap;
    typedef pair<string, SymbolTable::Symbol> SymbolMapEntry;

    SymbolMap class_scope_;
    SymbolMap subroutine_scope_;
    int static_index_;
    int field_index_;
    int arg_index_;
    int var_index_;
    string none_type_;

public:
    SymbolTable();

    // Starts a new subroutine scope, the existing subroutine scope, if any, is erased.
    void StartSubroutine();

    // Defines a new symbol of a give the identifier name, type, and kind.  The symbol is assigned a new
    // index.  STATIC and FIELD kinds have class scope, VAR and ARG kinds have subroutine scope.
    void Define(const string &name, const string &type, SymbolKind kind);

    // Returns the number of variables of a given kind.
    int KindCount(SymbolKind kind) const;

    // Returns the kind of the given symbol  or SymbolKind::NONE if the symbol is not
    // defined.
    SymbolKind KindOf(const string &name) const;

    // Returns the type of the given symbol or an empty string if the symbol is not
    // defined.
    const string &TypeOf(const string &name) const;

    // Returns the index of the given symbol or -1 if the symbol is not defined.
    int IndexOf(const string &name) const;
};

#endif
