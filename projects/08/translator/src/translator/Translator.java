package translator;

import java.io.File;
import java.io.FilenameFilter;

/**
 * Translates VM instructions into Hack assembly language.
 * @author kujawk
 *
 */
final public class Translator 
{
    /**
     * Runs the translator VM files.
     * @param args A single .vm file or a directory containing multiple .vm files.
     */
    public static void main(String args[])
    {
    	boolean outputLineNumbers = args.length == 2 ? true : false;
    	if (args.length != 1 && args.length != 2)
    	{
    		System.out.println("Usage: <input file> [outputLineNumbers]");
    		return;
    	}
    	File[] inputFiles = getInputFiles(args[0]);
    	File outputFile = getOutputFile(args[0]);
    	System.out.println("Output file: " + outputFile.getName());
    	try (CodeWriter codeWriter = new CodeWriter(outputFile, outputLineNumbers))
    	{
	        for (File inputFile : inputFiles)
	        {
	        	System.out.print("Processing file: " + inputFile.getName() + "...");
	        	codeWriter.setFileName(inputFile.getName());
	        	try (Parser parser = new Parser(inputFile))
				{
					while (parser.hasMoreCommands())
					{
						parser.advance();
						codeWriter.writeCommand(parser.command(), parser.arg1(), parser.arg2());
					}	
				}
	            System.out.println("Done");
	        }
    	}
    	catch (Exception e)
    	{
    		System.out.println("Error: " + e);
    		e.printStackTrace();
    	}
    }
    
    private static File getOutputFile(String fileOrDirectoryName)
    {
        File file = new File(fileOrDirectoryName);
        if (file.isDirectory())
        {
            return  new File(file, fileOrDirectoryName + ".asm");    
        }
        else
        {
            return  new File(fileOrDirectoryName.substring(0, fileOrDirectoryName.length() - 3) + ".asm");    
        }
    	
    }
    
    private static File[] getInputFiles(String fileOrDirectoryName)
    {
        File file = new File(fileOrDirectoryName);
        File[] files = null;
        if (file.isDirectory())
        {
            files = file.listFiles(new FilenameFilter () {
                @Override
                public boolean accept(File dir, String name)
                {
                    return name.endsWith(".vm");
                }
            });
        }
        else
        {
            files = new File[1];
            files[0]= file;
        }
        return files;
    }
}
