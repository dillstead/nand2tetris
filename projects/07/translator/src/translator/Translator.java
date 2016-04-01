package translator;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;

/**
 * Translates VM instructions into Hack assembly language.
 * @author kujawk
 *
 */
public class Translator 
{
    /**
     * Runs the translator VM files.
     * @param args A single .vm file or a directory containing multiple .vm files.
     */
    public static void main(String args[])
    {
    	// parser and code writer implement closable
    	
    	CodeWriter codeWriter = new CodeWriter(getOutputFileName(args[1]));
        String[] inputFileNames = getInputFileNames(args[1]);
        for (String inputFileName : inputFileNames)
        {
        	codeWriter.setFileName(inputFileName);
        	Parser parser = new Parser(inputFileName);
        	while (parser.hasMoreCommands())
        	{
        		parser.advance();
        		codeWriter.writeCommand(parser.command(), parser.arg1(), parser.arg2());
        	}
        }
        codeWriter.close();
    }
    
    private static String getOutputFileName(String fileOrDirectoryName)
    {
    	int i = fileOrDirectoryName.lastIndexOf(".vm");
    	return  i != -1 ? fileOrDirectoryName.substring(0, i) : fileOrDirectoryName;
    }
    
    private static String[] getInputFileNames(String fileOrDirectoryName)
    {
        File file = new File(fileOrDirectoryName);
        if (file.isDirectory())
        {
            return file.list(new FilenameFilter () {

                @Override
                public boolean accept(File dir, String name)
                {
                    return name.endsWith(".vm");
                }
            });
                   
        }
        else
        {
            String[] fileNames = new String[1];
            fileNames[0] = fileOrDirectoryName;
            return fileNames;
        }
    }
}
