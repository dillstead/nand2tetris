#ifndef ANALYZER_H_
#define ANALYZER_H_

/**
 * Compiles a Jack file.
 */

class Compiler
{
public:
    /**
     * Takes a Jack file and compiles it into a .vm file in the same directory.
     * Returns true if the compilation succeeded, false if not.
     */
    bool Compile(const string &filename);
};

#endif
