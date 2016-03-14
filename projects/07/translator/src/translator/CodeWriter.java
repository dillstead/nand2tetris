package translator;

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;

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
     * Writes the assembly code that is the translation of the given arithmetic command.
     * @param command Arithmetic command.
     */
    void writeArithmetic(String command)
    {
        
    }
    
    /**
     * Writes the assembly code that is the translation of push or pop.
     * @param command Push or pop.
     */
    void writePushPop(String command)
    {
        
    }
    
    /**
     * Closes the output file.
     * @throws IOException If an error ocurred closing the output file.
     */
    void close() throws IOException
    {
        writer.close();
    }
}
