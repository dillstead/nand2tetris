package translator;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Handles parsing of a single VM file.
 * @author kujawk
 *
 */
class Parser
{
    private final BufferedReader reader;
    private String[] tokens;
    private CommandType commandType;
    private String arg1;
    private String arg2;

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
    
    /**
     * Creates a parser for VM file.
     * @param inputFileName
     * @throws FileNotFoundException If the file is not found. 
     */
    Parser(String inputFileName) throws FileNotFoundException
    {
        reader = new BufferedReader(new FileReader(inputFileName));
    }
    
    /**
     * Checks to see if more commands) are available.    
     * @return True if there are more commands, false if not.
     * @throws IOException If a problem occurred reading from the file.
     */
    boolean hasMoreCommands() throws IOException
    {
        try
        {
            String line = reader.readLine();
            while (line != null)
            {
               tokens = line.split("\\s+");
               // Ignore blank lines and comments.
               if (tokens.length > 0 && tokens[0].charAt(0) != '/')
               {
                   return true;
               }
               line = reader.readLine();
            }
            reader.close();
            return false;
        }
        catch (IOException e)
        {
            reader.close();
            throw e;
        }
    }
    
    /**
     * Breaks down the current command into its constituent parts.
     */
    void advance()
    {
        commandType = commandTypeMap.get(tokens[0]);
        arg1 = tokens.length > 1 ? tokens[1] : null;
        arg2 = tokens.length > 2 ? tokens[2] : null;
        
        if (commandType == null)
        {
            throw new IllegalArgumentException("Invalid command " + arg1);
        }
    }
    
    /**
     * @return Type of the current command.
     */
    CommandType commandType()
    {
        return commandType;
    }
    
    /**
     * Returns the first argument of the current command, if the command 
     * has one.
     * @return The first argument of the current command.
     */
    String arg1()
    {
        return arg1;
    }
    
     /**
     * Returns the second argument of the current command, if the command 
     * has one.
     * @return The second argument of the current command.
     */
    String arg2()
    {
        return arg2;
    }
}
