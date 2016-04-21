package translator;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * Translates VM commands into Hack assembly code and writes the assembly
 * to the output file. 
 * @author kujawk
 *
 */
final class CodeWriter implements Closeable
{
    private final PrintWriter writer;
    private String inputFileName;
    private String currentFunction = "";
    private int labelCount;
    private int address = 0;
    
    private static final Map<String, String> segmentMap;
    private static final Set<String> realSegmentSet;
    private static final Set<String> virtualSegmentSet;
    private static final Set<String> constantSegmentSet;
    private static final Set<String> staticSegmentSet;
    private static final Set<String> pushCommandSet;
    private static final Set<String> popCommandSet;
    private static final Set<String> labelCommandSet;
    private static final Set<String> ifCommandSet;
    private static final Set<String> gotoCommandSet;
    private static final Set<String> callCommandSet;
    private static final Set<String> functionCommandSet;
    private static final Set<String> returnCommandSet;
    static
    {
    	Set<String> set = new HashSet<String>();
        set.add("local");
        set.add("argument");
        set.add("this");
        set.add("that");
        realSegmentSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("pointer");
        set.add("temp");
        virtualSegmentSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("constant");
        constantSegmentSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("static");
        staticSegmentSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("push");
        pushCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("pop");
        popCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("label");
        labelCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("goto");
        gotoCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("if-goto");
        ifCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("call");
        callCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("function");
        functionCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("return");
        returnCommandSet = Collections.unmodifiableSet(set);
        Map<String, String> map = new HashMap<String, String>();
        map.put("local", "LCL");
        map.put("argument", "ARG");
        map.put("this", "THIS");
        map.put("that", "THAT");
        map.put("pointer", "THIS");
        map.put("temp", "R5");
        segmentMap = Collections.unmodifiableMap(map);
    }
    private static final Map<String, String> arithmeticOperatorMap;
    private static final Set<String> twoArgArithmeticCommandSet;
    private static final Set<String> singleArgArithmeticCommandSet;
    private static final Set<String> twoArgArithmeticComparisonCommandSet;
    static
    {
        Set<String> set = new HashSet<String>();
        set.add("add");
        set.add("sub");
        set.add("and");
        set.add("or");
        twoArgArithmeticCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("neg");
        set.add("not");
        singleArgArithmeticCommandSet = Collections.unmodifiableSet(set);
        set = new HashSet<String>();
        set.add("gt");
        set.add("lt");
        set.add("eq");
        twoArgArithmeticComparisonCommandSet = Collections.unmodifiableSet(set);
        Map<String, String> map = new HashMap<String, String>();
        map.put("add", "+");
        map.put("sub", "-");
        map.put("and", "&");
        map.put("or", "|");
        map.put("neg", "-");
        map.put("not", "!");
        map.put("gt", "JGT");
        map.put("lt", "JLT");
        map.put("eq", "JEQ");
        arithmeticOperatorMap = Collections.unmodifiableMap(map);
    }
    
    /**
     * Creates a code writer.
     * @param outputFileName
     * @throws IOException If the file could not be opened.
     */
    CodeWriter(File outputFile) throws IOException
    {
    	writer = new PrintWriter(outputFile);
        //writeInitialization();
    }
    
    /**<
     * Informs the code writer that the translation of a new VM file has started.
     * @param inputFileName File name of the new VM file.
     */
    void setFileName(String inputFileName)
    {
        writeComment("New file " + inputFileName);
        this.inputFileName = inputFileName;
    }
    
    /**
     * Writes the assembly code that is the translation of the given command.
     * @param command Command
     * @param arg1 Optional first command argument.
     * @param arg2 Optional second command argument.
     * @throws IOException If writning to the output file failed.
     */
    void writeCommand(String command, String arg1, String arg2) throws IOException
    {
    	writeComment(command + (arg1 != null ? " " + arg1 : "") + (arg2 != null ? " " + arg2 : "")); 
    	if (twoArgArithmeticCommandSet.contains(command))
    	{
    		writeTwoArgArithmeticCommand(arithmeticOperatorMap.get(command));
    	}
    	else if (singleArgArithmeticCommandSet.contains(command))
    	{
    		writeSingleArgArithmeticCommand(arithmeticOperatorMap.get(command));
    	}
    	else if (twoArgArithmeticComparisonCommandSet.contains(command))
    	{
    		writeTwoArgArithmeticComparisonCommand(arithmeticOperatorMap.get(command));
    	}
        else if (pushCommandSet.contains(command))
        {
            if (realSegmentSet.contains(arg1))
            {
                writePushRealSegmentCommand(segmentMap.get(arg1), arg2);
            }
            else if (virtualSegmentSet.contains(arg1))
            {
                writePushVirtualSegmentCommand(segmentMap.get(arg1), arg2);
            }
            else if (constantSegmentSet.contains(arg1))
            {
                writePushConstantSegmentCommand(arg2);
            }
            else if (staticSegmentSet.contains(arg1))
            {
                writePushStaticSegmentCommand(arg2);
            }
            else
            {
            	throw new IllegalArgumentException("Invalid push segment: " + arg1);
            }
        }
        else if (popCommandSet.contains(command))
        {
            if (realSegmentSet.contains(arg1))
            {
                writePopRealSegmentCommand(segmentMap.get(arg1), arg2);
            }
            else if (virtualSegmentSet.contains(arg1))
            {
                writePopVirtualSegmentCommand(segmentMap.get(arg1), arg2);
            }
            else if (staticSegmentSet.contains(arg1))
            {
                writePopStaticSegmentCommand(segmentMap.get(arg1), arg2);
            }
            else
            {
            	throw new IllegalArgumentException("Invalid pop segment: " + arg1);
            }
        }
    	else if (labelCommandSet.contains(command))
    	{
    	    writeLabelCommand(arg1);
    	}
    	else if (gotoCommandSet.contains(command))
    	{
    	    writeGotoCommand(arg1);
    	}
    	else if (ifCommandSet.contains(command))
    	{
    	    writeIfCommand(arg1);
    	}
    	else if (callCommandSet.contains(command))
    	{
    	    writeCallCommand(arg1, arg2);
    	}
    	else if (functionCommandSet.contains(command))
    	{
    	    writeFunctionCommand(arg1, arg2);
    	}
    	else if (returnCommandSet.contains(command))
    	{
    	    writeReturnCommand();
    	}
        else
        {
            throw new IllegalArgumentException("Invalid command: " + command);
        }
    }
    
    /**
     * Closes the output file.
     * @throws IOException If an error occurred closing the output file.
     */
    public void close() throws IOException
    {
        writer.close();
    }
    
    private void writeTwoArgArithmeticCommand(String operator) throws IOException
    {
    	pop("D", "M");
    	pop("M", "M" + operator + "D");
    	push(null);
    }
    
    private void writeSingleArgArithmeticCommand(String operator) throws IOException
    {
    	pop("M", operator + "M");
    	push(null);
    }
    
    private void writeTwoArgArithmeticComparisonCommand(String operator) throws IOException
    {
    	String endLabel = makeLabel("END");
    	String operatorLabel = makeInternalLabel(operator);
        pop("D", "M");
    	pop("D", "M-D");
    	writeInstruction("@" + operatorLabel);
    	writeInstruction("D;" + operator);
    	writeInstruction("D=0");
    	writeInstruction("@" + endLabel);
    	writeInstruction("0;JMP");
    	writeLabel(operatorLabel);
    	writeInstruction("D=-1");
    	writeLabel(endLabel);
    	push("D");
    }

    private void writePushRealSegmentCommand(String segment, String offset) throws IOException
    {
    	writeInstruction("@" + segment);
    	writeInstruction("D=M");
    	writeInstruction("@" + offset);
    	writeInstruction("A=A+D");
    	writeInstruction("D=M");
    	push("D");
    }

    private void writePushVirtualSegmentCommand(String segment, String offset) throws IOException
    {
    	writeInstruction("@" + segment);
    	writeInstruction("D=A");
    	writeInstruction("@" + offset);
    	writeInstruction("A=A+D");
    	writeInstruction("D=M");
    	push("D");
    }

    private void writePushConstantSegmentCommand(String constant) throws IOException
    {
    	writeInstruction("@" + constant);
    	writeInstruction("D=A");
    	push("D");
    }

    private void writePushStaticSegmentCommand(String offset) throws IOException
    {
    	writeInstruction("@" + inputFileName + "." + offset);
    	writeInstruction("D=M");
    	push("D");
    }

    private void writePopRealSegmentCommand(String segment, String offset) throws IOException
    {
    	writeInstruction("@" + segment);
    	writeInstruction("D=M");
    	writeInstruction("@" + offset);
    	writeInstruction("D=D+A");
    	writeInstruction("@R13");
    	writeInstruction("M=D");
    	pop("D", "M");
    	writeInstruction("@R13");
    	writeInstruction("A=M");
    	writeInstruction("M=D");
    }

    private void writePopVirtualSegmentCommand(String segment, String offset) throws IOException
    {
    	writeInstruction("@" + segment);
    	writeInstruction("D=A");
    	writeInstruction("@" + offset);
    	writeInstruction("D=D+A");
    	writeInstruction("@R13");
    	writeInstruction("M=D");
    	pop("D", "M");
    	writeInstruction("@R13");
    	writeInstruction("A=M");
    	writeInstruction("M=D");
    }

    private void writePopStaticSegmentCommand(String segment, String offset) throws IOException
    {
    	pop("D", "M");
    	writeInstruction("@" + inputFileName + "." + offset);
    	writeInstruction("M=D");
    }
    
    private void writeLabelCommand(String label) throws IOException
    {
    	writeLabel(label);
    }
    
    private void writeGotoCommand(String label) throws IOException
    {
        writeInstruction("@" + makeLabel(label));
        writeInstruction("0;JMP");
    }
    
    private void writeIfCommand(String label) throws IOException
    {
        writeInstruction("@SP");
        writeInstruction("AM=M-1");
        writeInstruction("D=M");
        writeInstruction("@" + makeLabel(label));
        writeInstruction("D;JNE");
    }
    
    private void writeCallCommand(String function, String argumentCount) throws IOException
    {
        // Push return address.
        writeComment(" -- Push return address.");
        String returnLabel = makeInternalLabel("RET");
        writeInstruction("@" + returnLabel);
        writeInstruction("D=A");
        push("D");
        // Push LCL, ARG, THIS, and THAT.
        saveSegment("LCL");
        saveSegment("ARG");
        saveSegment("THIS");
        saveSegment("THAT");
        // ARG = SP - n - 5
        writeComment(" -- ARG = SP - n - 5");
        writeInstruction("@SP");
        writeInstruction("D=M");
        writeInstruction("@" + argumentCount);
        writeInstruction("D=D-A");
        writeInstruction("@5");
        writeInstruction("D=D-A");
        writeInstruction("@ARG");
        writeInstruction("M=D");
        // LCL = SP
        writeComment(" -- LCL = SP");
        writeInstruction("@SP");
        writeInstruction("D=M");
        writeInstruction("@LCL");
        writeInstruction("M=D");
        // Goto function.
        writeComment(" -- Goto function.");
        writeInstruction("@" + makeLabel(function));
        writeInstruction("M=D");
        // Return label.
        writeComment(" -- Return label.");
        writeLabel(returnLabel);
    }
    
    private void writeFunctionCommand(String function, String localCount) throws IOException
    {
        currentFunction = function;
        // Function label.
        writeComment(" -- Function label.");
        writeLabel(makeLabel(function));
        // Initialize all local variables to 0.
        int j = Integer.parseInt(localCount);
        for (int i = 0; i < j; i++)
        {
            writeComment(" -- Initialize local " + i + " to 0");
            writeInstruction("@0");
            writeInstruction("D=A");
            push("D");
        }
    }
    
    private void writeReturnCommand()
    {
        // FRAME (R13) = LCL
        writeComment(" -- FRAME (R13) = LCL");
        writeInstruction("@LCL");
        writeInstruction("D=M");
        writeInstruction("@R13");
        writeInstruction("M=D");
        // RET (R14) = *(FRAME - 5)
        writeComment(" -- RET (R14) = *(FRAME - 5)");
        writeInstruction("@5");
        writeInstruction("A=D-A");
        writeInstruction("D=M");
        writeInstruction("@R14");
        writeInstruction("M=D");
        // *ARG = pop()
        writeComment(" -- *ARG = pop()");
        pop("D", "M");
        writeInstruction("@ARG");
        writeInstruction("M=D");
        // SP = ARG + 1
        writeComment(" -- SP = ARG + 1");
        writeInstruction("D=A+1");
        writeInstruction("@SP");
        writeInstruction("M=D");
        // THAT, THIS, ARG, LOCAL = *(FRAME (R13) - (1..4))
        restoreSegment("THAT", "@R13", "1");
        restoreSegment("THIS", "@R13", "2");
        restoreSegment("ARG", "@R13", "3");
        restoreSegment("LOCAL", "@R13", "4");
        // Goto RET
        writeComment(" -- Goto RET");
        writeInstruction("@R14");
        writeInstruction("A=M");
        writeInstruction("0;JMP");
    }
    
    private void writeInitialization()
    {
        writeComment("Initialization");
        writeInstruction("@256");
        writeInstruction("D=A");
        writeInstruction("@SP");
        writeInstruction("M=D");
    }
    
    void saveSegment(String segment)
    {
        writeComment(" -- Push " + segment);
        writeInstruction("@" +  segment);
        writeInstruction("D=M");
        push("D");
    }
    
    void restoreSegment(String segment, String localVar, String offset)
    {
        writeComment(" -- " + segment + "= *(FRAME (R13) - " + offset + ")");
        writeInstruction(localVar);
        if (offset.equals("1"))
        {
            // Optimize for the fact that 1 can be subtracted in a single
            // instruction.
            writeInstruction("A=M-1");
        }
        else
        {
            writeInstruction("D=M");                
            writeInstruction("@" + offset);
            writeInstruction("A=D-A");
        }
        writeInstruction("D=M");
        writeInstruction("@" + segment);
        writeInstruction("M=D");
    }
    
    private void push(String comp)
    {
    	if (comp != null)
    	{
    		writeInstruction("@SP");
    		writeInstruction("A=M");
    		writeInstruction("M=" + comp);
    	}
    	writeInstruction("@SP");
    	writeInstruction("M=M+1");
    }
    
    private void pop(String dest, String comp)
    {
    	writeInstruction("@SP");
    	writeInstruction("AM=M-1");
    	writeInstruction(dest + "=" + comp);
    }
    
    private String makeInternalLabel(String label)
    {
    	// Ensure that the label is unique by appending a suffix.
    	return makeLabel(label) + "$" + Integer.toString(labelCount++);
    }
    
    private String makeLabel(String label)
    {
    	return currentFunction + "$" + label;
    }
    
    private void writeInstruction(String instruction)
    {
    	writeComment("ROM address " + address++);
    	writer.println(instruction);
    }
    
    private void writeLabel(String label)
    {
    	writer.println("(" + label + ")");
    }
    
    private void writeComment(String comment)
    {
    	writer.println("// " + comment);
    }
}
