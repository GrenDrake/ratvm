/* **************************************************************************
 * Origin Class Definition
 *
 * This Origin class is used to store the source file location something came
 * from.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#ifndef ORIGIN_H
#define ORIGIN_H

#include <string>

class Origin {
public:
    Origin()
    : file("(internal)"), fileNameString(-1), line(0), column(0)
    { }
    Origin(const std::string &file, int line, int column)
    : file(file), fileNameString(-1), line(line), column(column)
    { }

    std::string file;
    int fileNameString;
    int line, column;
};

std::ostream& operator<<(std::ostream &out, const Origin &origin);

#endif
