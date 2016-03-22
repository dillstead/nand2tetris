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
    private final BufferedWriter writer;
    private String inputFileName;
    
    private static final Map<CommandType, String> twoArgCommandMap;
    private static final Map<CommandType, String> singleArgCommandMap;
    static
    {
        Map<CommandType, String> map = new HashMap<CommandType, String>();
        map.put(CommandType.C_ADD, "+");
        map.put(CommandType.C_SUB, "-");
        map.put(CommandType.C_AND, "&");
        map.put(CommandType.C_OR, "|");
        twoArgCommandMap = Collections.unmodifiableMap(map);
        map  = new HashMap<CommandType, String>();
        map.put(CommandType.C_NEG, "-");
        map.put(CommandType.C_NOT, "!");
        singleArgCommandMap = Collections.unmodifiableMap(map);
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
    void writeCommand(CommandType command) throws IOException
    {
    	if (twoArgCommandMap.containsKey(command))
    	{
    		writeTwoArgCommand(twoArgCommandMap.get(command));
    	}
    	else if (singleArgCommandMap.containsKey(command))
    	{
    		writeSingleArgCommand(singleArgCommandMap.get(command));
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
    
    private void writeTwoArgCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append("D=M");
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append("M=M");
    	sb.append(operator);
    	sb.append("D");
    	sb.append("@SP");
    	sb.append("M=M+1");
    	writer.write(sb.toString());
    }
    
    private void writeSingleArgCommand(String operator) throws IOException
    {
    	StringBuilder sb = new StringBuilder();
    	sb.append("@SP");
    	sb.append("AM=M-1");
    	sb.append("M=");
    	sb.append(operator);
    	sb.append("M");
    	sb.append("D=M");
    	sb.append("@SP");
    	sb.append("M=M+1");
    	writer.write(sb.toString());
    }
}
