package translator;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

/**
 * Handles parsing of a single VM file.
 * @author kujawk
 *
 */
class Parser implements Closeable
{
    private final BufferedReader reader;
    private String[] tokens;
    private String command;
    private String arg1;
    private String arg2;
    
    /**
     * Creates a parser for VM file.
     * @param inputFileName
     * @throws FileNotFoundException If the file is not found. 
     */
    Parser(File inputFile) throws FileNotFoundException
    {
        reader = new BufferedReader(new FileReader(inputFile));
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
                if (tokens.length > 0 && !tokens[0].isEmpty() && tokens[0].charAt(0) != '/')
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
        command = tokens[0];
        arg1 = tokens.length > 1 ? tokens[1] : null;
        arg2 = tokens.length > 2 ? tokens[2] : null;
    }
    
    /**
     * @return Returns the current command.
     */
    String command()
    {
        return command;
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

	@Override
	public void close() throws IOException {
		// TODO Auto-generated method stub
	}
}
