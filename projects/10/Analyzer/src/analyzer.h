#ifndef ANALYZER_H_
#define ANALYZER_H_

/**
 * Analyzes a Jack file.
 */

class Analyzer
{
public:
    /**
     * Takes a Jack file and turns it into a XML "parse tree" file in the same directory.
     * Returns true if the analyze succeeded, false if not.
     */
    bool Analyze(const string &filename);
};

#endif
