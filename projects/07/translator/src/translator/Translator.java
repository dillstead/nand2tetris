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
        String[] fileNames = getFileNames(args[1]);
    }
    
    private static String[] getFileNames(String fileOrDirectoryName)
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
