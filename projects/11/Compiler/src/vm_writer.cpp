#include <vector>

#include "vm_writer.h"

static const string &SegmentName(Segment segment);
static const string &CommandName(Command command);

static vector<string> segment_names = {
    "constant",
    "argument",
    "local",
    "static",
    "this",
    "that",
    "pointer",
    "temp"
};

const string &SegmentName(Segment segment)
{
    return segment_names[int(segment)];
};

static vector<string> command_names = {
    "add",
    "sub",
    "neg",
    "eq",
    "gt",
    "lt",
    "and",
    "or",
    "not"
};

const string &CommandName(Command command)
{
    return command_names[int(command)];
};

VmWriter::VmWriter(ofstream &out) : out_(out)
{

}

void VmWriter::WritePush(Segment segment, int index)
{
    out_ << "push " << SegmentName(segment) << " " << index << endl;
}

void VmWriter::WritePop(Segment segment, int index)
{
    out_ << "pop " << SegmentName(segment) << " " << index << endl;
}

void VmWriter::WriteArithmetic(Command command)
{
    out_ << CommandName(command) << endl;
}

void VmWriter::WriteLabel(const string &label)
{
    out_ << "label " << label << endl;
}

void VmWriter::WriteGoto(const string &label)
{
    out_ << "goto " << label << endl;
}

void VmWriter::WriteIf(const string &label)
{
    out_ << "if-goto " << label << endl;
}

void VmWriter::WriteCall(const string &name, int num_args)
{
    out_ << "call " << name << " " << num_args << endl;
}

void VmWriter::WriteFunction(const string &name, int num_locals)
{
    out_ << "function " << name << " " << num_locals << endl;
}

void VmWriter::WriteReturn()
{
    out_ << "return" << endl;
}
