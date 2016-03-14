package translator;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

/**
 * Handles parsing of a single VM file.
 * @author kujawk
 *
 */
class Parser
{
    enum CommandType
    {
        C_ARITHMETIC,
        C_PUSH,
        C_POP
    }
    
    private final BufferedReader reader;
    private String[] tokens;
    private CommandType commandType;
    private String arg1;
    private String arg2;
    
    private static final Set<String> arithmeticCommands = new HashSet<String>();
    private static final Set<String> pushCommand  = new HashSet<String>();
    private static final Set<String> popCommand  = new HashSet<String>();
    
    static
    {
        arithmeticCommands.addAll(Arrays.asList("add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"));
        pushCommand.addAll(Arrays.asList("push"));
        popCommand .addAll(Arrays.asList("push"));
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
        arg1 = tokens[0];
        if (arithmeticCommands.contains(arg1))
        {
             commandType = CommandType.C_ARITHMETIC;
        }
        else if (pushCommand.contains(arg1))
        {
            commandType = CommandType.C_PUSH;
            arg2 = tokens[1];
        }
        else if (popCommand.contains(arg1))
        {
            commandType = CommandType.C_POP;
            arg2 = tokens[1];
        }
        else
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
     * Returns the first argument of the current command.  In the case of 
     * CommandTypee.C_ARITHMETIC the command itself (add, sub, etc.) is
     * returned.
     * @return The first argument of the current command.
     */
    String arg1()
    {
        return arg1;
    }
    
    /**
     * Returns the second  argument of the current command.  Should be
     * called only if the current command is C_PUSH or C_POP.
     * @return The second argument of the current command.
     */
    String arg2()
    {
        return arg2;
    }
}
