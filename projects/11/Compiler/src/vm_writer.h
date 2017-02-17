#ifndef VMWRITER_H_
#define VMWRITER_H_

#include <fstream>
using namespace std;

enum class Segment
{
    CONSTANT,
    ARGUMENT,
    LOCAL,
    STATIC,
    THIS,
    THAT,
    POINTER,
    TEMP
};

enum class Command
{
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT
};

// Emits VM commands into a file, using the VM command syntax.
class VmWriter
{
    ofstream &out_;

public:
    VmWriter(ofstream &out);

    void WritePush(Segment segment, int index);
    void WritePop(Segment segment, int index);
    void WriteArithmetic(Command command);
    void WriteLabel(const string &label);
    void WriteGoto(const string &label);
    void WriteIf(const string &label);
    void WriteCall(const string &name, int num_args);
    void WriteFunction(const string &name, int num_locals);
    void WriteReturn();
};

#endif
