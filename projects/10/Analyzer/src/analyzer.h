#ifndef ANALYZER_H_
#define ANALYZER_H_

/**
 * Analyzes a jack file or directory.
 */

class Analyzer
{
public:
    /**
     * Takes a Jack file and tokenizes and parses it.  A resulting XML "parse tree" file will be
     * generated in the same directory.  Returns true if the analyze succeeded, false if not.
     */
    bool Analyze(string &filename);
};

#endif
