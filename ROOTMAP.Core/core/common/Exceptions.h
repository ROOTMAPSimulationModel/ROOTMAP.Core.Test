/*
    Exceptions.h

    Author:            Robert van Hugten
    Description:    <describe the StringConversionException class here>
*/

#ifndef Exceptions_H
#define Exceptions_H

#include <exception>
#include <string>
#include "core/macos_compatibility/macos_compatibility.h"
#include "wx/string.h"

namespace rootmap
{
    //#define ROOTMAP_EXCEPTION_INLINE_FUNCTIONS
#if defined ROOTMAP_EXCEPTION_INLINE_FUNCTIONS
    class RmException : public std::exception
    {
    public:
        RmException(const char * reason)
            : myReason(reason) {}
        RmException(wxString& reason)
            : myReason(reason) {}
        virtual ~RmException() {}
        virtual const char * what() const { return myReason.c_str(); }
    private:
        std::string myReason;

        RmException(const RmException &) {}
        RmException& operator=(const RmException &) { return *this; }
    };
#else // NOT #if defined ROOTMAP_EXCEPTION_INLINE_FUNCTIONS
    class RmException : public std::exception
    {
    public:
        RmException(const char* reason);
        RmException(wxString& reason);
        RmException(const RmException&);
        virtual ~RmException();
        virtual const char* what() const;
    private:
        std::string myReason;

        RmException& operator=(const RmException&);
    };
#endif // NOT #if defined ROOTMAP_EXCEPTION_INLINE_FUNCTIONS

    class StringConversionException : public RmException
    {
    public:
        StringConversionException(const char* reason, const char* bad_string);
        StringConversionException(const char* reason, const std::string& bad_string);

        virtual ~StringConversionException()
        {
        }

        virtual const char* what() const;

        StringConversionException(const StringConversionException&);
    private:
        std::string m_badString;

        StringConversionException& operator=(const StringConversionException&);
    };

    class UnimplementedFunctionException : public RmException
    {
    public:
        UnimplementedFunctionException(const char* reason);
        UnimplementedFunctionException(const UnimplementedFunctionException& original);
    private:
        UnimplementedFunctionException& operator=(const UnimplementedFunctionException&);
    };
} /* namespace rootmap */

#endif    // Exceptions_H
