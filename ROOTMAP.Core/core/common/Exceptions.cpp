/*
    CropSimExceptions.cpp

    Author:            Robert van Hugten
    Description:    <describe the StringConversionException class here>
*/

#include "core/common/Exceptions.h"

namespace rootmap
{
#if !defined ROOTMAP_EXCEPTION_INLINE_FUNCTIONS

    RmException::RmException(const char* reason)
        : myReason(reason)
    {
    }

    RmException::RmException(wxString& reason)
        : myReason(reason)
    {
    }

    RmException::RmException(const RmException& original)
        : myReason(original.myReason)
    {
        //throw std::logic_error("RmException copy constructor disabled");
    }

    RmException& RmException::operator=(const RmException& /* rhs */)
    {
        throw std::logic_error("RmException assignment operator disabled");
    }

    RmException::~RmException()
    {
    }

    const char* RmException::what() const
    {
        return myReason.c_str();
    }
#endif // #if !defined ROOTMAP_EXCEPTION_INLINE_FUNCTIONS

    StringConversionException::StringConversionException(const char* reason, const char* bad_string)
        : RmException(reason)
    {
        m_badString = std::string(reason) + std::string(" : ") + std::string(bad_string);
    }

    StringConversionException::StringConversionException(const char* reason, const std::string& bad_string)
        : RmException(reason)
    {
        m_badString = std::string(reason) + std::string(" : ") + std::string(bad_string);
    }

    StringConversionException::StringConversionException(const StringConversionException& original)
        : RmException(original)
        , m_badString(original.m_badString)
    {
    }

    const char* StringConversionException::what() const
    {
        return m_badString.c_str();
    };

    UnimplementedFunctionException::UnimplementedFunctionException(const char* reason)
        : RmException(reason)
    {
    }

    UnimplementedFunctionException::UnimplementedFunctionException(const UnimplementedFunctionException& original)
        : RmException(original)
    {
    }
} /* namespace rootmap */
