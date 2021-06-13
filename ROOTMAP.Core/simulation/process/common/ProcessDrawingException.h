/* 
    ProcessDrawingException.h

    Author:            Robert van Hugten
    Description:    <describe the StringConversionException class here>
*/

#ifndef ProcessDrawingException_H
#define ProcessDrawingException_H

#include "core/common/Exceptions.h"

namespace rootmap
{

    class ProcessDrawingException : public RmException
    {
    public:
        ProcessDrawingException(const char * reason)
            : RmException(reason) {}
        ProcessDrawingException(const ProcessDrawingException & original)
            : RmException(original.what()) {}
        ProcessDrawingException& operator=(const ProcessDrawingException &) { return *this; }
    };


} /* namespace rootmap */

#endif    // ProcessDrawingException_H
