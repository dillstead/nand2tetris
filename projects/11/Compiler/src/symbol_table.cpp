#include <vector>

#include "symbol_table.h"

static vector<string> kind_names = {
    "none",
    "static",
    "field",
    "arg",
    "var",
};

const string &SymbolKindName(SymbolKind kind)
{
    return kind_names[int(kind)];
};

SymbolTable::SymbolTable() : static_index_(0), field_index_(0), arg_index_(0), var_index_(0)
{

}

void SymbolTable::StartSubroutine()
{
    subroutine_scope_.clear();
    arg_index_ = 0;
    var_index_ = 0;
}

void SymbolTable::Define(const string &name, const string &type, SymbolKind kind)
{
    switch (kind)
    {
    case SymbolKind::STATIC:
    {
        SymbolMapEntry symbol(name, SymbolTable::Symbol(name, type, kind, static_index_++));
        class_scope_.insert(symbol);
        break;
    }
    case SymbolKind::FIELD:
    {
        SymbolMapEntry symbol(name, SymbolTable::Symbol(name, type, kind, field_index_++));
        class_scope_.insert(symbol);
        break;
    }
    case SymbolKind::ARG:
    {
        SymbolMapEntry symbol(name, SymbolTable::Symbol(name, type, kind, arg_index_++));
        subroutine_scope_.insert(symbol);
        break;
    }
    case SymbolKind::VAR:
    {
        SymbolMapEntry symbol(name, SymbolTable::Symbol(name, type, kind, var_index_++));
        subroutine_scope_.insert(symbol);
        break;
    }
    default:
    {
        break;
    }
    }
}

int SymbolTable::KindCount(SymbolKind kind) const
{
    int count = 0;

    switch (kind)
    {
    case SymbolKind::STATIC:
    {
        count = static_index_ + 1;
        break;
    }
    case SymbolKind::FIELD:
    {
        count = field_index_ + 1;
        break;
    }
    case SymbolKind::ARG:
    {
        count = arg_index_ + 1;
        break;
    }
    case SymbolKind::VAR:
    {
        count = var_index_ + 1;
        break;
    }
    default:
    {
        break;
    }
    }

    return count;
}

SymbolKind SymbolTable::KindOf(const string &name) const
{
    if (subroutine_scope_.count(name) > 0)
    {
        return subroutine_scope_.at(name).kind_;
    }
    else if (class_scope_.count(name) > 0)
    {
        return class_scope_.at(name).kind_;
    }
    else
    {
        return SymbolKind::NONE;
    }
}

const string &SymbolTable::TypeOf(const string &name) const
{
    if (subroutine_scope_.count(name) > 0)
    {
        return subroutine_scope_.at(name).type_;
    }
    else if (class_scope_.count(name) > 0)
    {
        return class_scope_.at(name).type_;
    }
    else
    {
        return none_type_;
    }
}

int SymbolTable::IndexOf(const string &name) const
{
    if (subroutine_scope_.count(name) > 0)
    {
        return subroutine_scope_.at(name).index_;
    }
    else if (class_scope_.count(name) > 0)
    {
        return class_scope_.at(name).index_;
    }
    else
    {
        return -1;
    }
}
