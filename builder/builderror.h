/* **************************************************************************
 * BuildError Exception declaration
 *
 * The BuildError class repersents exception thrown by the compiler due to
 * errors in the build process.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/

#ifndef BUILDERROR_H
#define BUILDERROR_H

#include <stdexcept>
#include <string>

#include "origin.h"

class BuildError : public std::runtime_error {
public:
    BuildError(const std::string &msg);
    BuildError(const Origin &origin, const std::string &msg);
    virtual const char* what() const throw();
    const std::string& getMessage() const;
    const Origin& getOrigin() const;
private:
    Origin mRawOrigin;
    std::string mRawMessage;
};

#endif
