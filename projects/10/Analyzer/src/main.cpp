#include <dirent.h>
#include <string.h>
#include <iostream>
using namespace std;

#include "analyzer.h"

static bool is_jack_file(const char *filename);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Usage " << argv[0] << " <Jack file or directory>" << endl;
        exit(1);
    }

    Analyzer analyzer;

    if (is_jack_file(argv[1]))
    {
        string filename("./");

        analyzer.Analyze(filename.append(argv[1]));
    }
    else
    {
        if (DIR *pdir = opendir(argv[1]))
        {
            while (struct dirent *pdirent = readdir(pdir))
            {
                if (is_jack_file(pdirent->d_name))
                {
                    string filename(argv[1]);

                    analyzer.Analyze(filename.append("/").append(pdirent->d_name));
                }
            }
        }
    }

    return 0;
}

bool is_jack_file(const char *filename)
{
    if (const char *ext = strrchr((filename), '.'))
    {
        return (strcmp(ext, ".jack") == 0);
    }

    return false;
}
