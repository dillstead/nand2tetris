package translator;

import java.io.BufferedWriter;
import java.io.Closeable;
import java.io.FileWriter;
import java.io.IOException;
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
class CodeWriter implements Closeable
{
    private final BufferedWriter writer;
    private String inputFileName;
    
    private static final Map<String, String> segmentMap;
    private static final Set<String> realSegmentSet;
    private static final Set<String> virtualSegmentSet;
    private static final Set<String> constantSegmentSet;
    private static final Set<String> staticSegmentSet;
    private static final Set<String> pushCommandSet;
    private static final Set<String> popCommandSet;
    static
    {
    	Set<String> set = new HashSet<String>();
        set.add("local");
        set.add("argument");
        set.add("this");
        set.add("that");
        realSegmentSet = Collections.unmodifiableSet(set);
        set.clear();
        set.add("pointer");
        set.add("temp");
        virtualSegmentSet = Collections.unmodifiableSet(set);
        set.clear();
        set.add("constant");
        constantSegmentSet = Collections.unmodifiableSet(set);
        set.clear();
        set.add("static");
        staticSegmentSet = Collections.unmodifiableSet(set);
        set.clear();
        set.add("push");
        pushCommandSet = Collections.unmodifiableSet(set);
        set.clear();
        set.add("pop");
        popCommandSet = Collections.unmodifiableSet(set);
        Map<String, String> map = new HashMap<String, String>();
        map.put("local", "LCL");
        map.put("argument", "ARG");
        map.put("this", "THIS");
        map.put("that", "THAT");
        map.put("pointer", "THIS");
        map.put("temp", "RAM5");
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
        set.clear();
        set.add("neg");
        set.add("not");
        singleArgArithmeticCommandSet = Collections.unmodifiableSet(set);
        set.clear();
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
    CodeWriter(String outputFileName) throws IOException
    {
        writer = new BufferedWriter(new FileWriter(outputFileName));
    }
    
    /**<
     * Informs the code writer that the translation of a new VM file has started.
     * @param inputFileName File name of the new VM file.
     */
    void setFileName(String inputFileName)
    {
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
    	StringBuilder sb = new StringBuilder();
    	sb.append(pop("D", "M"));
    	sb.append(pop("M", "M" + operator + "D"));
    	sb.append(push(null));
    	writer.write(sb.toString());
    }
    
    private void writeSingleArgArithmeticCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append(pop("M", operator + "M"));
    	sb.append(push(null));
    	writer.write(sb.toString());
    }
    
    private void writeTwoArgArithmeticComparisonCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append(pop("D", "M"));
    	sb.append(pop("D", "M-D"));
    	sb.append("@" + operator);
    	sb.append("D;" + operator);
    	sb.append("D=0");
    	sb.append("@END");
    	sb.append("0;JMP");
    	sb.append("(" + operator + ")");
    	sb.append("D=-1");
    	sb.append("(END)");
    	sb.append(push("D"));
    	writer.write(sb.toString());
    }

    private void writePushRealSegmentCommand(String segment, String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + segment);
    	sb.append("D=M");
    	sb.append("@" + offset);
    	sb.append("A=A+D");
    	sb.append("D=M");
    	sb.append(push("D"));
    	writer.write(sb.toString());
    }

    private void writePushVirtualSegmentCommand(String segment, String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + segment);
    	sb.append("D=A");
    	sb.append("@" + offset);
    	sb.append("A=A+D");
    	sb.append("D=M");
    	sb.append(push("D"));
    	writer.write(sb.toString());
    }

    private void writePushConstantSegmentCommand(String constant) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + constant);
    	sb.append("D=A");
    	sb.append(push("D"));
    	writer.write(sb.toString());
    }

    private void writePushStaticSegmentCommand(String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + inputFileName + "." + offset);
    	sb.append("D=M");
    	sb.append(push("D"));
    	writer.write(sb.toString());
    }

    private void writePopRealSegmentCommand(String segment, String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + segment);
    	sb.append("D=M");
    	sb.append("@" + offset);
    	sb.append("D=D+A");
    	sb.append("@R13");
    	sb.append("M=D");
    	sb.append(pop("D", "M"));
    	sb.append("@R13");
    	sb.append("A=M");
    	sb.append("M=D");
    }

    private void writePopVirtualSegmentCommand(String segment, String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@" + segment);
    	sb.append("D=A");
    	sb.append("@" + offset);
    	sb.append("D=D+A");
    	sb.append("@R13");
    	sb.append("M=D");
    	sb.append(pop("D", "M"));
    	sb.append("@R13");
    	sb.append("A=M");
    	sb.append("M=D");
    }

    private void writePopStaticSegmentCommand(String segment, String offset) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append(pop("D", "M"));
    	sb.append("@" + inputFileName + "." + offset);
    	sb.append("M=D");
    	writer.write(sb.toString());
    }
    
    private String push(String comp)
    {
    	StringBuilder sb = new StringBuilder();
    	
    	if (comp != null)
    	{
    		sb.append("@SP");
    		sb.append("A=M");
    		sb.append("M=D");
    	}
    	sb.append("@SP");
    	sb.append("M=M+1");
    	return sb.toString();
    }
    
    private String pop(String dest, String comp)
    {
    	StringBuilder sb = new StringBuilder();
    	
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append(dest + "=" + comp);
    	return sb.toString();
    }
}
