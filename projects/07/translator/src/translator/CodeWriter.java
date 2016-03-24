package translator;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Translates VM commands into Hack assembly code and writes the assembly
 * to the output file. 
 * @author kujawk
 *
 */
class CodeWriter
{
	private enum CommandType
	{
		C_ADD,
	    C_SUB,
	    C_NEG,
	    C_EQ,
	    C_GT,
	    C_LT,
	    C_AND,
	    C_OR,
	    C_NOT,
	    C_PUSH,
	    C_POP
	}
	
	private static final Map<String, CommandType> commandTypeMap;
    static
    {
        Map<String, CommandType> map = new HashMap<String, CommandType>();
        map.put("add", CommandType.C_ADD);
        map.put("sub", CommandType.C_SUB);
        map.put("add", CommandType.C_NEG);
        map.put("add", CommandType.C_EQ);
        map.put("add", CommandType.C_GT);
        map.put("add", CommandType.C_LT);
        map.put("add", CommandType.C_AND);
        map.put("add", CommandType.C_OR);
        map.put("add", CommandType.C_NOT);
        map.put("add", CommandType.C_PUSH);
        map.put("add", CommandType.C_POP);
        commandTypeMap = Collections.unmodifiableMap(map);
    }
	
    // break down push into virtual, regular, constant, static
    // virtual map segment to hack variable, use virtual push template with hack variable and index
    // regular map segment to hack variable, use virtual pop template
    // constant map, use constant template
    // static map, use static template
    
    // break down pop in same fashion
    
    // single command map to hack operator -> single template
    // double command map to hack operator -> double template
    // comparison command map to hack operator -> comparison template
    
    private final BufferedWriter writer;
    private String inputFileName;
    
    private static final Map<CommandType, String> twoArgArithmeticCommandMap;
    private static final Map<CommandType, String> singleArgArithmeticCommandMap;
    private static final Map<CommandType, String> twoArgArithmeticComparisonCommandMap;

    static
    {
        Map<CommandType, String> map = new HashMap<CommandType, String>();
        map.put(CommandType.C_ADD, "+");
        map.put(CommandType.C_SUB, "-");
        map.put(CommandType.C_AND, "&");
        map.put(CommandType.C_OR, "|");
        twoArgArithmeticCommandMap = Collections.unmodifiableMap(map);
        map.clear();
        map.put(CommandType.C_NEG, "-");
        map.put(CommandType.C_NOT, "!");
        singleArgArithmeticCommandMap = Collections.unmodifiableMap(map);
        map.clear();
        map.put(CommandType.C_GT, "JGT");
        map.put(CommandType.C_LT, "JLT");
        map.put(CommandType.C_EQ, "JEQ");
        twoArgArithmeticComparisonCommandMap = Collections.unmodifiableMap(map);
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
    
    /**
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
     * @throws IOException If writing to the output file failed.
     */
    void writeCommand(String command) throws IOException
    {
    	CommandType commandType = commandTypeMap.get(command);
    	if (commandType == null)
    	{
    		throw new IllegalArgumentException("Invalid command: " + command);
    	}
    	if (twoArgArithmeticCommandMap.containsKey(command))
    	{
    		writeTwoArgArithmeticCommand(twoArgArithmeticCommandMap.get(command));
    	}
    	else if (singleArgArithmeticCommandMap.containsKey(command))
    	{
    		writeSingleArgArithmeticCommand(singleArgArithmeticCommandMap.get(command));
    	}
    	else if (twoArgArithmeticComparisonCommandMap.containsKey(command))
    	{
    		writeTwoArgArithmeticComparisonCommand(twoArgArithmeticCommandMap.get(command));
    	}
    }
    
    /**
     * Closes the output file.
     * @throws IOException If an error occurred closing the output file.
     */
    void close() throws IOException
    {
        writer.close();
    }
    
    private void writeTwoArgArithmeticCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	// Pop stack into D.
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append("D=M");
    	sb.append("@SP");
    	// Pop stack and place address into A.
    	sb.append("AM=M-1");
    	// Operate on D and M, store result in M (top of stack).
    	sb.append("M=M");
    	sb.append(operator);
    	sb.append("D");
    	// Increment SP.
    	sb.append("@SP");
    	sb.append("M=M+1");
    	writer.write(sb.toString());
    }
    
    private void writeSingleArgArithmeticCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	// Pop stack and place address into A.
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	// Operate on M, store result in M (top of stack).
    	sb.append("M=");
    	sb.append(operator);
    	sb.append("M");
    	sb.append("D=M");
    	// Increment SP.
    	sb.append("@SP");
    	sb.append("M=M+1");
    	writer.write(sb.toString());
    }
    
    private void writeTwoArgArithmeticComparisonCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	// Pop stack into D.
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append("D=M");
    	// Pop stack and place address into A.
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	// Compute difference of M and D and store in D.
    	sb.append("D=M-D");
    	// Jump according to the operator.
    	sb.append("@");
    	sb.append(operator);
    	sb.append("D;");
    	sb.append(operator);
    	// D is 0 if successful.
    	sb.append("D=0");
    	// Finished.
    	sb.append("@END");
    	sb.append("0;JMP");
    	
    	
    	sb.append(operator);
    	sb.append("D");
    	sb.append("@SP");
    	sb.append("M=M+1");
    	writer.write(sb.toString());
    }
}
